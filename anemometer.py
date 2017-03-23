"""
Demonstrates reading a single analog input (AIN) from a LabJack.

"""

from labjack import ljm
import time

# Open first found LabJack
handle = ljm.open(ljm.constants.dtANY, ljm.constants.ctANY, "ANY")
#handle = ljm.openS("ANY", "ANY", "ANY")

info = ljm.getHandleInfo(handle)
print("Opened a LabJack with Device type: %i, Connection type: %i,\n" \
    "Serial number: %i, IP address: %s, Port: %i,\nMax bytes per MB: %i" % \
    (info[0], info[1], info[2], ljm.numberToIP(info[3]), info[4], info[5]))

while True:
	# Setup and call eReadName to read from a AIN on the LabJack.
	name = "AIN1"
	result = ljm.eReadName(handle, name)
	mps_conversion = 5.08 # meters per second conversion
	fpm_conversion = 1000 # feet per minute conversion
	output_range = 5 # anemometer gives up to 5V output

	# voltage range of output
	wind = (mps_conversion*result)/output_range

	print("%s Anemometer: %f m/s" % (name, wind))
	# slow down readings
	time.sleep(.5);

# Close handle
ljm.close(handle)
