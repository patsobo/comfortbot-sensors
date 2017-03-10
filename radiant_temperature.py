"""
Demonstrates reading a single analog input (AIN) from a LabJack.

"""

from labjack import ljm

R1=3000
Vs=4.915
STD_TEMP=25
# Open first found LabJack
handle = ljm.open(ljm.constants.dtANY, ljm.constants.ctANY, "ANY")
#handle = ljm.openS("ANY", "ANY", "ANY")

info = ljm.getHandleInfo(handle)
print("Opened a LabJack with Device type: %i, Connection type: %i,\n" \
    "Serial number: %i, IP address: %s, Port: %i,\nMax bytes per MB: %i" % \
    (info[0], info[1], info[2], ljm.numberToIP(info[3]), info[4], info[5]))

while True:
	# Setup and call eReadName to read from a AIN on the LabJack.
	name0 = "AIN0"
	result0 = ljm.eReadName(handle, name0)
	name1 = "AIN1"
	result1 = ljm.eReadName(handle, name1)
	name2 = "AIN2"
	result2 = ljm.eReadName(handle, name2)

	# 0  voltage range of output
	resistance = float((100*result0)/(4.915 - result0))
	rad_temp = 100*(resistance - 100) / 38.51
	print("%s: %f degrees Celsius" % (name0, rad_temp))

	# 1  voltage range of output
	humidity = (100*result1)/4.915

	print("%s Humidity: %f %%" % (name1, humidity))
	# slow down readings
	#time.sleep(100);

	# 2  voltage range of output
	Ro = float((R1*result2)/(Vs - result2))
	amb_temp = R1 / Ro * 25 #R1*(Ro - R1) / STD_TEMP 

	print("%s: %f degrees Celsius" % (name2, amb_temp))

	

# Close handle
ljm.close(handle)
