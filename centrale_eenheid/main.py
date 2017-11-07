#https://arduino-er.blogspot.nl/2015/04/python-to-plot-graph-of-serial-data.html
import serial
import matplotlib.pyplot as plt
from drawnow import *
import atexit

<<<<<<< HEAD
serialArduino = serial.Serial('COM5', 9600)

print("serialArduino.isOpen() = " + str(serialArduino.isOpen()))
=======
lux_values = []
temp_values = []

plt.ion()
cnt = 0

serialArduino = serial.Serial('COM5', 19200)

def plotValues():
    plt.title("Values from light and temperature")
    plt.grid(True)
    plt.ylabel('Values')
    plt.subplot(211)
    plt.plot(lux_values, 'rx-', label='values')
    plt.subplot(212)
    plt.plot(temp_values, 'rx-', label='values')
    plt.show()

# pre-load dummy data
for i in range(0, 26):
    lux_values.append(0)
    temp_values.append(0)
>>>>>>> master

while True:
    while (serialArduino.inWaiting() == 0):
        pass
<<<<<<< HEAD
    print("readline()")
    valueRead = serialArduino.readline(1)
    print(int(valueRead.hex(), 16))
=======
    valueRead = serialArduino.readline()

    # check if valid value can be casted
    try:
        valueRead = valueRead.split()
        print(valueRead[0])
        if 'L' in valueRead[0].decode("utf-8"): #lux
            valueInInt = float(valueRead[1])
            print(valueInInt)
            if valueInInt > 0:
                if valueInInt >= 0:
                    lux_values.append(valueInInt)
                    lux_values.pop(0)
                    drawnow(plotValues)
                else:
                    print("Invalid! negative number")
            else:
                print("Invalid! too large")
        if 'T' in  valueRead[0].decode("utf-8"): #temp
            valueInInt = float(valueRead[1])
            print(valueInInt)
            if valueInInt > 0:
                if valueInInt >= 0:
                    temp_values.append(valueInInt)
                    temp_values.pop(0)
                    drawnow(plotValues)
                else:
                    print("Invalid! negative number")
            else:
                print("Invalid! too large")
    except ValueError:
        print("Invalid! cannot cast")
>>>>>>> master
