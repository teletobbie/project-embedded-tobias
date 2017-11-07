#https://www.youtube.com/watch?v=8eM0qpSuIlw
import serial
import np
import matplotlib.pyplot as plt
from drawnow import *
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

values = []

plt.ion()
cnt = 0

serialArdef plotValues():
    plt.title('Serial value from Arduino')
    plt.grid(True)
    plt.ylabel('Values')
    plt.plot(values, 'rx-', label='values')
    plt.legend(loc='upper right')duino = serial.Serial('COM5', 19200)




# pre-load dummy data
for i in range(0, 26):
    values.append(0)

while True:
    while (serialArduino.inWaiting() == 0):
        pass
    valueRead = serialArduino.readline()

    # check if valid value can be casted
    try:
        valueRead = valueRead.split()
        print(valueRead[0])
        if valueRead[0] == 'L':
            valueInInt = float(valueRead[1])
            print(valueInInt)
            if valueInInt > 0:
                if valueInInt >= 0:
                    values.append(valueInInt)
                    values.pop(0)
                    plt.title("Value from light")
                    drawnow(plotValues)
                else:
                    print("Invalid! negative number")
            else:
                print("Invalid! too large")
        if valueRead[0] == "T":
            valueInInt = float(valueRead[1])
            print(valueInInt)
            if valueInInt > 0:
                if valueInInt >= 0:
                    values.append(valueInInt)
                    values.pop(0)
                    plt.title("Value from temperature")
                    drawnow(plotValues)
                else:
                    print("Invalid! negative number")
            else:
                print("Invalid! too large")

    except ValueError:
        print("Invalid! cannot cast")
