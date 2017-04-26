"""
Reading all sensors voltage, convert value, send to DB

"""

import random
import sys
import json
import time
import argparse
import thread
import threading
import traceback

from labjack import ljm
from ws4py.client.threadedclient import WebSocketClient
from cmd import Cmd

DDP_VERSIONS = ["pre1"]

def log(msg):
    """A shortcut to write to the standard error file descriptor"""
    sys.stderr.write('{}\n'.format(msg))


def parse_command(params):
    """Parses a command with a first string param and a second
    json-encoded param"""
    name, args = (params + ' ').split(' ', 1)
    return name, args and json.loads(args) or []


class DDPClient(WebSocketClient):
    """simple wrapper around Websockets for DDP connections"""
    def __init__(self, url, print_raw):
        WebSocketClient.__init__(self, url)
        self.print_raw = print_raw

        # We keep track of methods and subs that have been sent from the
        # client so that we only return to the prompt or quit the app
        # once we get back all the results from the server.
        #
        # `id`
        #
        #   The operation id, informed by the client and returned by the
        #   server to make sure both are talking about the same thing.
        #
        # `result_acked`
        #
        #   Flag to make sure we were answered.
        #
        # `data_acked`
        #
        #   Flag to make sure we received the correct data from the
        #   message we were waiting for.
        self.pending_condition = threading.Condition()
        self.pending = {}


    def send(self, msg_dict):
        """Send a message through the websocket client and wait for the
        answer if the message being sent contains an id attribute.
        Also prints to the standard error fd.
        (NOTE: DDP does not require waiting for an answer but this is
        a simple proof-of-concept client)"""
        message = json.dumps(msg_dict)
        if self.print_raw:
            log('[RAW] >> {}'.format(message))
        super(DDPClient, self).send(message)

        # We don't need to wait for certain messages, just for the ones
        # with ids.
        if 'id' in msg_dict:
            self.block_until_return(msg_dict['msg'], msg_dict['id'])

    def block_until_return(self, msg_type, msg_id):
        """Wait until the msg_id that was sent to the server is answered"""
        with self.pending_condition:
            self.pending = {'id': msg_id}

            while self.pending.get('id') is not None:
                if msg_type == 'method':
                    # Methods must validate both data and result flag
                    we_are_good = all((
                            self.pending.get('result_acked'),
                            self.pending.get('data_acked')))
                else:
                    # Subs just need to validate data flag
                    we_are_good = self.pending.get('data_acked')

                if we_are_good:
                    return
                self.pending_condition.wait()

    def opened(self):
        """Set the connecte flag to true and send the connect message to
        the server."""
        self.send({"msg": "connect", "version": DDP_VERSIONS[0],
                   "support": DDP_VERSIONS})

    def received_message(self, data):
        """Parse an incoming message and print it. Also update
        self.pending appropriately"""
        if self.print_raw:
            log('[RAW] << {}'.format(data))

        msg = json.loads(str(data))

        changed_pending = False


        with self.pending_condition:
            if msg.get('msg') == 'error':
                log("* ERROR {}".format(msg['reason']))
                # Reset all pending state
                self.pending = {}
                changed_pending = True

            elif msg.get('msg') == 'connected':
                log("* CONNECTED")

            elif msg.get('msg') == 'failed':
                log("* FAILED; suggested version {}".encode('ascii', errors='ignore').format(msg['version']))

            elif msg.get('msg') == 'result':
                if msg['id'] == self.pending.get('id'):
                    if msg.get('result'):
                        log("* METHOD RESULT {}".encode('ascii', errors='ignore').format(msg['result']))
                    elif msg.get('error'):
                        log("* ERROR {}".encode('ascii', errors='ignore').format(msg['error']['reason']))
                    else:
                        log("* METHOD FINISHED")
                    self.pending.update({'result_acked': True})
                    changed_pending = True

            elif msg.get('msg') == 'added':
                log("* ADDED {} {}".encode('ascii', errors='ignore').format(
                        msg['collection'], msg['id']))
                if 'fields' in msg:
                    for key, value in msg['fields'].items():
                        log("  - FIELD {} {}".format(key, value))
            elif msg.get('msg') == 'changed':
                log("* CHANGED {} {}".encode('ascii', errors='ignore').format(
                        msg['collection'], msg['id']))
                if 'fields' in msg:
                    for key, value in msg['fields'].items():
                        log("  - FIELD {} {}".encode('ascii', errors='ignore').format(key, value))
                if 'cleared' in msg:
                    for key in msg['cleared']:
                        log("  - CLEARED {}".encode('ascii', errors='ignore').format(key));
            elif msg.get('msg') == 'removed':
                pass
#                log("* REMOVED {} {}".encode('ascii', errors='ignore').format(
#                        msg['collection'], ", ".join(msg['ids'])))
            elif msg.get('msg') == 'ready':
                assert 'subs' in msg
                if self.pending.get('id') in msg['subs']:
                    log("* READY")
                    self.pending.update({'data_acked': True})
                    changed_pending = True
            elif msg.get('msg') == 'updated':
                if self.pending.get('id') in msg['methods']:
                    log("* UPDATED")
                    self.pending.update({'data_acked': True})
                    changed_pending = True
            elif msg.get('msg') == 'nosub':
                log("* NOSUB")
                self.pending.update({'data_acked': True})
                changed_pending = True

            if changed_pending:
                self.pending_condition.notify()


    def closed(self, code, reason=None):
        """Called when the connection is closed"""
        log('* CONNECTION CLOSED {} {}'.format(code, reason))

    # Overrides WebSocket to run to ensure that if an unhandled exception is
    # thrown in the thread, we print the exception and *then* kill the main
    # thread.
    def run(self):
        try:
            super(DDPClient, self).run()
        except:
            traceback.print_exc()
        finally:
            with self.pending_condition:
                self.pending_condition.notify()
            thread.interrupt_main()


"""Parse the command line arguments and create a new App instance"""
parser = argparse.ArgumentParser(
    description='A command-line tool for communicating with a DDP server.')
parser.add_argument(
        'ddp_endpoint', metavar='ddp_endpoint',
        help='DDP websocket endpoint to connect ' +
        'to, e.g. madewith.meteor.com')
parser.add_argument(
        '--print-raw', dest='print_raw', action="store_true",
        help='print raw websocket data in addition to parsed results')
args = parser.parse_args()

ddpclient = DDPClient(
        'ws://' + args.ddp_endpoint + '/websocket',
        args.print_raw)
ddpclient.connect()


#fake location dimension
xDim=8
yDim=9

#remove old datapoints
room_json ="[]"
command = "deleteData " + room_json #[\"testing sending json directly\"]"
method_name, params = parse_command(command)

unique_id = 0

# send BBW280 info
ddpclient.send({
    "msg": "method",
    "method": method_name,
    "params": params,
    "id": str(unique_id)
})
unique_id += 1

# This is the websocket client that will actually talk with
# meteor
#ddp_endpoint = "162.243.45.179"
room_json = "[{\"name\": \"BBW281\", \"x\": " + str(xDim) + ", \"y\": " + str(yDim) + "}]"
command = "insertRoom " + room_json #[\"testing sending json directly\"]"
method_name, params = parse_command(command)

# send BBW280 info
ddpclient.send({
    "msg": "method",
    "method": method_name,
    "params": params,
    "id": str(unique_id)
})
unique_id += 1


R1=3000
Vs=4.915
STD_TEMP=25

# Constants for Anemometer
mps_conversion = 5.08 # meters per second conversion
fpm_conversion = 1000 # feet per minute conversion
output_range = 5 # anemometer gives up to 5V output

# Open first found LabJack
handle = ljm.open(ljm.constants.dtANY, ljm.constants.ctANY, "ANY")
#handle = ljm.openS("ANY", "ANY", "ANY")

info = ljm.getHandleInfo(handle)
iterator=0
radiantList=[]
humidityList=[]
ambientList=[]
anemo1List=[]
anemo2List=[]

print("Opened a LabJack with Device type: %i, Connection type: %i,\n" \
    "Serial number: %i, IP address: %s, Port: %i,\nMax bytes per MB: %i" % \
    (info[0], info[1], info[2], ljm.numberToIP(info[3]), info[4], info[5]))

#location starter
x=0
y=0

while True:

    # Setup and call eReadName to read from a AIN on the LabJack.
    # 0=radiant, 1=humidity, 2=ambient, 3=4=anemometers
    name0 = "AIN12"
    name1 = "AIN1"
    name2 = "AIN13"
    name3 = "AIN2"
    name4 = "AIN3"
    result0 = ljm.eReadName(handle, name0)
    result1 = ljm.eReadName(handle, name1)
    result2 = ljm.eReadName(handle, name2)
    result3 = ljm.eReadName(handle, name3)
    result4 = ljm.eReadName(handle, name4)

    # 0  radiant temp voltage range of output
    resistance = float((100*result0)/(4.915 - result0))
    rad_temp = 100*(resistance - 100) / 38.51
    radiantList.append(rad_temp)
    print("%s Radiant temp:     %f deg C" % (name0, rad_temp))

    # 1  humidity voltage range of output
    humidity = (100*result1)/4.915
    humidityList.append(humidity)
    print("%s  Humidity:     %f %%" % (name1, humidity))

    # 2  ambient temp voltage range of output
    Ro = float((R1*result2)/(Vs - result2))
    amb_temp = R1 / Ro * STD_TEMP
    ambientList.append(amb_temp)
    print("%s Ambient temp:    %f deg C " % (name2, amb_temp))

    # 3  anemometer number 1: Setup and call eReadName to read from a AIN on the LabJack.
    # voltage range of output
    wind1 = (mps_conversion*result3)/output_range
    anemo1List.append(wind1)
    print("%s  Wind1:         %f m/s" % (name3, wind1))

    # 4  anemometer number 2: Setup and call eReadName to read from a AIN on the LabJack.
    # voltage range of output
    wind2 = (mps_conversion*result4)/output_range
    anemo2List.append(wind2)
    print("%s  Wind2:         %f m/s" % (name4, wind2))
    print("--------------------------------")
    iterator+=1    
    print(iterator)
    print("--------------------------------")

    # slow down readings
    time.sleep(0.5);

    if iterator >= 20:

        #avg the values
        radAvg=sum(radiantList)/len(radiantList)
        hudAvg=sum(humidityList)/len(humidityList)
        ambAvg=sum(ambientList)/len(ambientList)
        ane1Avg=sum(anemo1List)/len(anemo1List)
        ane2Avg=sum(anemo2List)/len(anemo2List)
        aneMagAvg=pow((pow(ane1Avg,2)+pow(ane2Avg,2)),0.5)
        print("radAvg= %f" % (radAvg))
        print("hudAvg= %f" % (hudAvg))
        print("ambAvg= %f" % (ambAvg))
        print("ane1Avg= %f" % (ane1Avg))
        print("ane2Avg= %f" % (ane2Avg))
        print("aneMagnitude= %f" % (aneMagAvg))
        time.sleep(5)
        
        #to-do: send avg values to DB
        command = "insertData [\"BBW281\", " + \
            "{\"x\":" + str(x) + \
            ",\"y\":" + str(y) + \
            ",\"temp\":" + str(ambAvg) + \
            ",\"radtemp\":" + str(radAvg) + \
            ",\"humid\":" + str(hudAvg) + \
            ",\"velocity\":" + str(aneMagAvg) + "}]"
        
        if x<xDim:
            x=x+1
        else:
            x=0
            y=y+1
            if y>=yDim:
                y=0
        #will need to use real data eventually, right now we increment fake loc
        

            
        #command = "insertData [{\"loc\":\"loc[q]\",\"temp\":\"temp[q]\",\"radtemp\":\"radtemp[q]\",\"humid\":\"humid[q]\",\"velocity\":\"velocity[q]\"}]"
        #print command
        method_name, params = parse_command(command)
         
        ddpclient.send({
            "msg": "method",
            "method": method_name,
            "params": params,
            "id": str(unique_id)
        })
        unique_id += 1
        #reset lists and iterator
        radiantList=[]
        humidityList=[]
        ambientList=[]
        anemo1List=[]
        anemo2List=[]
        iterator=0
            

# Close handle
ljm.close(handle)
