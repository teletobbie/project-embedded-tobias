#blue = v5
#gray = gnd
#black = gnd
#white = a0
#yellow = a1

#https://arduino-er.blogspot.nl/2015/04/python-to-plot-graph-of-serial-data.html
import serial
import np
import matplotlib.pyplot as plt
from drawnow import *
<<<<<<< HEAD
from matplotlib.widgets import Button
=======
"""
box = dict(facecolor='yellow', pad=5, alpha=0.2)

fig = plt.figure()
fig.subplots_adjust(left=0.2, wspace=0.6)

# Fixing random state for reproducibility
np.random.seed(19645801)

ax1 = fig.add_subplot(221)
ax1.plot(2000*np.random.rand(10))
ax1.set_title('ylabels not aligned')
ax1.set_ylabel('misaligned 1', bbox=box)
ax1.set_ylim(0, 2000)
ax3 = fig.add_subplot(223)
ax3.set_ylabel('misaligned 2',bbox=box)
ax3.plot(np.random.rand(10))


labelx = -0.3  # axes coords

ax2 = fig.add_subplot(222)
ax2.set_title('ylabels aligned')
ax2.plot(2000*np.random.rand(10))
ax2.set_ylabel('aligned 1', bbox=box)
ax2.yaxis.set_label_coords(labelx, 0.5)
ax2.set_ylim(0, 2000)

ax4 = fig.add_subplot(224)
ax4.plot(np.random.rand(10))
ax4.set_ylabel('aligned 2', bbox=box)
ax4.yaxis.set_label_coords(labelx, 0.5)


plt.show()
"""
>>>>>>> master

lux_values = []
temp_values = []
current_temp = 20
plt.ion()

<<<<<<< HEAD
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
<<<<<<< HEAD
    plt.subplots_adjust(hspace=.5)
    axButtonMin = plt.axes([0.7, 0.01, 0.1, 0.050])
    axButtonMax = plt.axes([0.88, 0.01, 0.1, 0.050])
    buttonMin = Button(axButtonMin, 'Min')
    buttonMin.on_clicked(callback.min)
    buttonMax = Button(axButtonMax, 'Max')
    buttonMax.on_clicked(callback.max)
    plt.gcf().text(0.83, 0.025, str(callback.getTemp), fontsize=10)

def read(valueRead):
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

def write(message):
    serial.write(bytes(message))

=======
    plt.show()
=======
serialArdef plotValues():
    plt.title('Serial value from Arduino')
    plt.grid(True)
    plt.ylabel('Values')
    plt.plot(values, 'rx-', label='values')
    plt.legend(loc='upper right')duino = serial.Serial('COM5', 19200)



>>>>>>> refs/remotes/origin/Jesse
>>>>>>> master

# pre-load dummy data
for i in range(0, 26):
    lux_values.append(0)
    temp_values.append(0)

while True:
    while (serialArduino.inWaiting() == 0):
        pass
    print("readline()")
    valueRead = serialArduino.readline()
    read(valueRead)

