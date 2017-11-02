#https://arduino-er.blogspot.nl/2015/04/python-to-plot-graph-of-serial-data.html
import serial
import matplotlib.pyplot as plt
from drawnow import *
import atexit

serialArduino = serial.Serial('COM5', 9600)

print("serialArduino.isOpen() = " + str(serialArduino.isOpen()))

while True:
    while (serialArduino.inWaiting() == 0):
        pass
    print("readline()")
    valueRead = serialArduino.readline(1)
    print(int(valueRead.hex(), 16))
