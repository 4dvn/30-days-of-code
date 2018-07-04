#!/usr/bin/python

# Commands
# Number of bytes returned in ()
CMD_READ          = 0x01   # Read data from specified address, valid locations 1 to 33 (5)

# Status/error return codes
ERR_OK           = 0x01    # No errors


class Unbuffered(object):
   def __init__(self, stream):
       self.stream = stream
   def write(self, data):
       self.stream.write(data)
       self.stream.flush()
   def __getattr__(self, attr):
       return getattr(self.stream, attr)
      
import sys
import serial

# change port name to match your particular instance
port = "/dev/tty.usbserial-A72D19NZ"    # OS X
#port = "//./COM3"                      # Windows

ser = serial.Serial(port, 9600, timeout = None)    # open serial port
sys.stdout = Unbuffered(sys.stdout)                # open stdout in unbuffered mode (automatically flush after each print operation)


msg = ''
print "Reading tag's memory contents:"
for ix in range(3, 32):
  while True:
    ser.write('!RW' + chr(CMD_READ) + chr(ix))    # send command
    buf = ser.read(5)           # get bytes (will block until received)
    if buf[0] == chr(ERR_OK):   # if valid data received with no error, continue
      break
  print "%2d" % ix, ": ", buf[1:]
  msg += buf[1:]

print "Secret Message:\n" , msg


ser.close()
