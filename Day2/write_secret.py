#!/usr/bin/python


# Commands
# Number of bytes returned in ()
CMD_READ          = 0x01   # Read data from specified address, valid locations 1 to 33 (5)
CMD_WRITE         = 0x02   # Write data to specified address, valid locations 3 to 31 (1)

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

message = raw_input("Input secret (Trimmed to 116 bytes):\n") # get secret message
message = message[0:116].ljust(116) # trim and pad message to 116 bytes exactly

print "Writing data to tag:"
for ix in range (3, 32):
  while True:
    line = message[(ix-3)*4:((ix-3)*4)+4] # ix is data vals 3-32, we want a 4-byte offset from (0-29)
    ser.write('!RW' + chr(CMD_WRITE) + chr(ix) + line)  # send command, write 4-byte string into each User EEPROM address
    buf = ser.read(1)           # get byte (will block until received)
    if buf[0] == chr(ERR_OK):   # if no error, continue
      break
  print "%2d" % ix, ":", line


ser.close()
