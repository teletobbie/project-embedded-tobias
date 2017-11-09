#blue = v5
#gray = gnd
#black = gnd
#white = a0
#yellow = a1

#https://arduino-er.blogspot.nl/2015/04/python-to-plot-graph-of-serial-data.html
import serial
import matplotlib.pyplot as plt
from drawnow import *
from matplotlib.widgets import Button

lux_values = []
temp_values = []
current_temp = 20
plt.ion()

serialArduino = serial.Serial('COM5', 19200)

class Index(object):
    def __init__(self, temp):
        self.current_temp = temp
    def max(self, event):
        self.current_temp += 1
    def min(self, event):
        self.current_temp -= 1
    def getTemp(self):
        return str(self.current_temp)

def plotValues():
    callback = Index(current_temp)
    plt.suptitle('Waarde van licht en temperatuur')
    plt.grid(True)
    plt.subplot(211)
    plt.subplot(211).set_xlabel('Lichtintensiteit in Lux')
    plt.plot(lux_values, 'rx-', label='values')
    plt.subplot(212)
    plt.subplot(212).set_xlabel('Temperatuur in °C')
    plt.plot(temp_values, 'rx-', label='values')
    plt.subplots_adjust(hspace=.5)
    axButtonMin = plt.axes([0.7, 0.01, 0.1, 0.050])
    axButtonMax = plt.axes([0.88, 0.01, 0.1, 0.050])
    buttonMin = Button(axButtonMin, 'Min')
    buttonMin.on_clicked(callback.min)
    buttonMax = Button(axButtonMax, 'Max')
    buttonMax.on_clicked(callback.max)
    plt.gcf().text(0.83, 0.025, str(callback.getTemp), fontsize=10)

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
        if 'L' in valueRead[0].decode("utf-8"):
            luxValueInFloat = float(valueRead[1])
            if luxValueInFloat >= 0:
                lux_values.append(luxValueInFloat)
                lux_values.pop(0)
        if 'T' in valueRead[0].decode("utf-8"):
            tempValueInFloat = float(valueRead[1])
            if tempValueInFloat >= 0:
                temp_values.append(tempValueInFloat)
                temp_values.pop(0)
        drawnow(plotValues)
    except ValueError:
        print("Invalid! cannot cast")
