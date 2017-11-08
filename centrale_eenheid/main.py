#blue = v5
#gray = gnd
#black = gnd
#white = a0
#yellow = a1

#https://arduino-er.blogspot.nl/2015/04/python-to-plot-graph-of-serial-data.html
import serial
import matplotlib.pyplot as plt
from drawnow import *
import numpy as np
from matplotlib.widgets import Slider

lux_values = []
temp_values = []
plt.ion()
serialArduino = serial.Serial('COM5', 19200)

def plotValues():
    x_lux = np.array([])
    plt.title("Values from light and temperature")
    plt.grid(True)
    plt.subplot(211)
    plt.subplot(211).set_xlabel('Lichtintensiteit in Lux')
    plt.plot(lux_values, 'rx-', label='values')
    plt.subplot(212)
    plt.subplot(212).set_xlabel('Temperatuur in °C')
    plt.plot(temp_values, 'rx-', label='values')
    plt.subplots_adjust(hspace=.4)
    



# pre-load dummy data
for i in range(0, 26):
    lux_values.append(0)
    temp_values.append(0)

while True:
    while (serialArduino.inWaiting() == 0):
        pass

    print("readline()")
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

