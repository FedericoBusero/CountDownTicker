from tkinter import *
from tkinter import ttk
from tkinter import font

import time
import datetime
import serial
import string

global endTime
global clock_mode
global deltaTime
global clockStatus

def quit(*args):
    ticker_updateStatus("",4)
    root.destroy()

def pretty_time_delta(seconds):
    sign_string = '-' if seconds < 0 else ''
    seconds = abs(int(seconds))
    days, seconds = divmod(seconds, 86400)
    hours, seconds = divmod(seconds, 3600)
    minutes, seconds = divmod(seconds, 60)
    if days > 0:
        return '%s%02dd%02d:%02d:%02d' % (sign_string, days, hours, minutes, seconds)
    elif hours > 0:
        return '%s%02d:%02d:%02d' % (sign_string, hours, minutes, seconds)
    else:
        return '%s%02d:%02d' % (sign_string, minutes, seconds)

def ticker_StartCountdown(hour, minutes, seconds):
    global endTime
    global deltaTime
    global clock_mode
    print("ticker_StartCountdown")
    clock_mode = 1
    deltaTime = datetime.timedelta(0,(3600*hour) + (60*minutes) + seconds,0,999)
    endTime = datetime.datetime.now() + deltaTime

def ticker_ResetCountdown(hour, minutes, seconds):
    global endTime
    global deltaTime
    global clock_mode
    print("ticker_ResetCountdown")
    clock_mode = 2
    deltaTime = datetime.timedelta(0,(3600*hour) + (60*minutes) + seconds,0,999)
    endTime = datetime.datetime.now() + deltaTime

def ticker_pause(*args):
    print("ticker_pause")
    global clockStatus
    if clock_mode == 1:
        ticker_updateStatus(clockStatus,2)
    elif clock_mode == 2:
        ticker_updateStatus(clockStatus,1)

def ticker_stop(*args):
    print("ticker_stop")
    ticker_updateStatus("",3)

def ticker_updateStatus(tijdstr,newmode):
    global clockStatus
    global clock_mode
    if ((tijdstr != clockStatus) |(newmode != clock_mode)):
        clockStatus = tijdstr
        clock_mode = newmode
        # print("current mode: ")
        # print(tijdstr)
        ser.write(tijdstr.encode())
        ser.write(b',')
        ser.write(str(clock_mode).encode())
        ser.write(b'\n')

def resetticker(*args):
    ticker_StartCountdown(0,0,7)

def resettickerpause(*args):
    ticker_ResetCountdown(0,0,12)

def processSerial():
    print("processSerial")
    line = ser.readline().decode('ascii')
    print(line)
    words = line.split()
    if words[0] == "Pause":
        print("Pause")
        ticker_pause()
    elif words[0] == "Stop":
        print("Stop")
        ticker_stop()
    elif words[0] == "StartCountdown":
        print("StartCountdown")
        ticker_StartCountdown(int(words[1]),int(words[2]),int(words[3]))
    elif words[0] == "ResetCountdown":
        print("ResetCountdown")
        ticker_ResetCountdown(int(words[1]),int(words[2]),int(words[3]))

def show_time():
    global deltaTime
    global endTime
    
    # print("showtime: ")
    # print(endTime)
    if (clock_mode == 1):
        # Get the time remaining until the event
        deltaTime = endTime - datetime.datetime.now()
    
    if (clock_mode == 2):
        # Get the time remaining until the event
        endTime = datetime.datetime.now() + deltaTime

    # remove the microseconds part
    ms = deltaTime.microseconds
    deltaTime = deltaTime - datetime.timedelta(microseconds=deltaTime.microseconds)
    # print(pretty_time_delta(deltaTime.total_seconds()))
    if deltaTime.total_seconds()>2*60:
        lbl.config(foreground="green")
    else:
        lbl.config(foreground="red")
    if deltaTime.total_seconds()<0:
        if ms<300000:
            lbl.config(foreground="black")

    if (clock_mode == 3):
        lbl.config(foreground="black")
    else:
        # Show the time left
        timestr = pretty_time_delta(deltaTime.total_seconds())
        ticker_updateStatus(timestr,clock_mode)
        txt.set(timestr)
    #process content on serial port
    moreBytes = ser.inWaiting()
    if moreBytes:
        processSerial()
        
    # Trigger the countdown after 1000ms
    root.after(100, show_time)


# Use tkinter lib for showing the clock
root = Tk()
root.attributes("-fullscreen", True)
root.configure(background='black')
root.bind("x", quit)
root.bind("r", resetticker)
root.bind("e", resettickerpause)
root.bind("s", ticker_stop)
root.bind("p", ticker_pause)
root.after(1000, show_time)

fnt = font.Font(family='Helvetica', size=200, weight='bold')
txt = StringVar()
lbl = ttk.Label(root, textvariable=txt, font=fnt, foreground="green", background="black")
lbl.place(relx=0.5, rely=0.5, anchor=CENTER)

#ser = serial.Serial("/dev/ttyUSB0",115200)
ser = serial.Serial("COM5",115200)
ser.close()
ser.open()

clockStatus=""
clock_mode = 0
# 0 = booting
# 1 = running
# 2 = paused
# 3 = stopped
# 4 = quit

# Set the end date and time for the countdown
#ticker_StartCountdown(0,12,0)
deltaTime = datetime.timedelta(0,0,0,0)
endTime = datetime.datetime.now()
ticker_updateStatus("",3)

root.mainloop()

