import serial
from tkinter import *
from collections import Counter
import time

class adrinoTerminal:
    def __init__(self):
        userInterface()

class adrinoSerial:

    def __init__(self, portname):
        try:
            self.ser = serial.Serial(portname, 19200, timeout = 0.1)
            #self.handshake()
        except:
            print("error")


    def handshake(self):
        self.ser.write(b'hello')
        while 1:
            if self.ser.readline():
                break

    #def

class userInterface:

    def __init__(self):
        self.window = Tk()
        self.window.title("Adrino")
        self.Infoframe = Frame(self.window)
        self.Infoframe.pack()
        #self.openConnection('COM4')
        self.tem = StringVar()
        self.lux = StringVar()
        self.dis = StringVar()
        self.COMPORT = StringVar()
        self.openDistance = StringVar()
        self.portstate = 0
        self.createConnectframe()

        self.mainLoop()

    def createConnectframe(self):
        self.connectFrame = Frame(self.window)
        self.connectFrame.pack()
        self.connectFrameTitleLabel = Label(self.connectFrame, text='Enter COMPORT')
        self.connectFrameTitleLabel.pack(side=TOP)
        self.connectFrameEntry = Entry(self.connectFrame, width=40, textvariable=self.COMPORT)
        self.connectFrameEntry.pack(side=LEFT)
        self.connectFrameButton = Button(self.connectFrame, text='Submit', command=self.openConnection)
        self.connectFrameButton.pack(side=LEFT)


    def removeConnectframe(self):
        self.connectFrame.pack_forget()


    def createInfoframe(self):
        self.Infoframe = Frame(self.window)
        self.Infoframe.pack()
        Label(self.Infoframe, text="Temp:").pack(side=LEFT)
        t = Label(self.Infoframe, textvariable=str(self.tem))
        t.pack(side=LEFT)
        Label(self.Infoframe, text="Lux: ").pack(side=LEFT)
        l = Label(self.Infoframe, textvariable=str(self.lux))
        l.pack(side=LEFT)
        Label(self.Infoframe, text="distance: ").pack(side=LEFT)
        d = Label(self.Infoframe, textvariable=str(self.dis))
        d.pack(side=LEFT)

    def createDistanceFrame(self):
        Label(self.Infoframe, text="Enter open distance: ").pack(side=LEFT)
        Entry(self.Infoframe, width=40, textvariable=self.openDistance).pack(side=LEFT)
        Button(self.Infoframe, text="Submit", command=self.writeDistance).pack(side=LEFT)

    def mainLoop(self):
        while 1:
            self.window.update_idletasks()
            self.window.update()
            if self.portstate > 0:
                self.updateVars()
            #self.updateVars()
            #time.sleep(1/1000)
            #self.window.mainloop()


    def openConnection(self):
        self.portname = str.upper(self.COMPORT.get())
        self.adrinoConnection = adrinoSerial(self.portname)
        self.portstate = 1
        self.removeConnectframe()
        self.createInfoframe()
        self.createDistanceFrame()


    def updateVars(self):

        if self.adrinoConnection.ser.inWaiting == 0:
            return None
        self.valueRead = self.adrinoConnection.ser.readline().rstrip().decode("utf-8")
        #print (self.valueRead)
        if 'T' in self.valueRead:
            self.tem.set(self.valueRead[1:])
        if 'L' in self.valueRead:
            self.lux.set(self.valueRead[1:])
        if 'D' in self.valueRead:
            self.dis.set(self.valueRead[1:])


    def writeDistance(self):
        self.writeDis = int(str(self.openDistance.get()))
        print(self.writeDis)


centrale = adrinoTerminal()
