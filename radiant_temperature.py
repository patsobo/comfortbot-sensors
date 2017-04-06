"""
Demonstrates reading a single analog input (AIN) from a LabJack.

"""

from labjack import ljm
import time

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
	print("%s Radiant temp: 	%f deg C" % (name0, rad_temp))

	# 1  humidity voltage range of output
	humidity = (100*result1)/4.915
	humidityList.append(humidity)
	print("%s  Humidity: 	%f %%" % (name1, humidity))

	# 2  ambient temp voltage range of output
	Ro = float((R1*result2)/(Vs - result2))
	amb_temp = R1 / Ro * 25 #R1*(Ro - R1) / STD_TEMP
	ambientList.append(amb_temp)
	print("%s Ambient temp:	%f deg C " % (name2, amb_temp))

	# 3  anemometer number 1: Setup and call eReadName to read from a AIN on the LabJack.
	# voltage range of output
	wind1 = (mps_conversion*result3)/output_range
	anemo1List.append(wind1)
	print("%s  Wind1: 		%f m/s" % (name3, wind1))

	# 4  anemometer number 2: Setup and call eReadName to read from a AIN on the LabJack.
	# voltage range of output
	wind2 = (mps_conversion*result4)/output_range
	anemo2List.append(wind2)
	print("%s  Wind2: 		%f m/s" % (name4, wind2))
	print("--------------------------------")
	iterator+=1	
	print(iterator)
	print("--------------------------------")

	# slow down readings (readings per second)
	time.sleep(1);

	if iterator >=20:
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

	#reset lists and iterator
		radiantList=[]
		humidityList=[]
		ambientList=[]
		anemo1List=[]
		anemo2List=[]
		iterator=0

# Close handle
ljm.close(handle)
