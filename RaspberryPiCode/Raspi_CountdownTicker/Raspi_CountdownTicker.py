import time
import datetime
import serial
import string
import platform
import os
import pygame
import sys

global endTime
global clock_mode
global deltaTime
global clockStatus
global currentdisplaycolour
global currentdisplaytime

def debug_print(textstr):
    # print(textstr)
    # ser.write(textstr.encode())
    # ser.write(b'\n')
    pass
        
def pretty_time_delta(seconds):
    sign_string = '-' if seconds < 0 else ''
    seconds = abs(int(seconds))
    days, seconds = divmod(seconds, 86400)
    hours, seconds = divmod(seconds, 3600)
    minutes, seconds = divmod(seconds, 60)
    if days > 0:
        return '%s%02dd%02d:%02d:%02d' % (sign_string, days, hours, minutes, seconds)
    elif hours > 0:
#        return '%s%02d:%02d:%02d' % (sign_string, hours, minutes, seconds)
        return '%s%d:%02d:%02d' % (sign_string, hours, minutes, seconds)
    else:
        return '%s%02d:%02d' % (sign_string, minutes, seconds)

def ticker_quit(*args):
    # debug_print("ticker_quit")
    ticker_updateStatus("",4)
    if (platform.system()=="Windows"):
        quit()
    else:
        quit()

def ticker_shutdown(*args):
    # debug_print("ticker_shutdown")
    ticker_updateStatus("",4)
    if (platform.system()=="Windows"):
        quit()
    else:
        os.system("sudo shutdown -h now")
        quit()

def ticker_StartCountdown(hour, minutes, seconds):
    global endTime
    global deltaTime
    global clock_mode
    # debug_print("ticker_StartCountdown")
    clock_mode = 1
    deltaTime = datetime.timedelta(0,(3600*hour) + (60*minutes) + seconds,0,999)
    endTime = datetime.datetime.now() + deltaTime

def ticker_ResetCountdown(hour, minutes, seconds):
    global endTime
    global deltaTime
    global clock_mode
    # debug_print("ticker_ResetCountdown")
    clock_mode = 2
    deltaTime = datetime.timedelta(0,(3600*hour) + (60*minutes) + seconds,0,999)
    endTime = datetime.datetime.now() + deltaTime

def ticker_test(*args):
    global endTime
    global deltaTime
    global clock_mode
    # debug_print("ticker_test")
    clock_mode = 1
    deltaTime = -datetime.timedelta(0,(3600*9) + (60*59) + 50,0,999)
    endTime = datetime.datetime.now() + deltaTime

def ticker_pause(*args):
    # debug_print("ticker_pause")
    global clockStatus
    if clock_mode == 1:
        ticker_updateStatus(clockStatus,2)
    elif clock_mode == 2:
        ticker_updateStatus(clockStatus,1)

def ticker_stop(*args):
    # debug_print("ticker_stop")
    ticker_updateStatus("",3)

def ticker_updateStatus(tijdstr,newmode):
    global clockStatus
    global clock_mode
    if ((tijdstr != clockStatus) |(newmode != clock_mode)):
        clockStatus = tijdstr
        clock_mode = newmode
        # print("current mode: ")
        # print(clock_mode)
        # print(tijdstr)
        ser.write(tijdstr.encode())
        ser.write(b',')
        ser.write(str(clock_mode).encode())
        ser.write(b'\n')

def ticker_ping():
    global clockStatus
    global clock_mode
    # print("Ping ")
    # print(clock_mode)
    # print(clockStatus)
    ser.write(clockStatus.encode())
    ser.write(b',')
    ser.write(str(clock_mode).encode())
    ser.write(b'\n')

def resetticker(*args):
    ticker_StartCountdown(0,0,7)

def resettickerpause(*args):
    ticker_ResetCountdown(0,0,12)

def processSerial():
    # debug_print("processSerial")
    try:
        line = ser.readline().decode('ascii')
    except UnicodeDecodeError:
        #debug_print("UnicodeDecodeError")
        line = ""
    else:
        pass
    # debug_print(line)
    words = line.split()
    if (len(words)>0):
        if words[0] == "Pause":
            # print("Pause")
            ticker_pause()
        elif words[0] == "Stop":
            # print("Stop")
            ticker_stop()
        elif words[0] == "Quit":
            # print("Quit")
            ticker_shutdown()
        elif words[0] == "Ping":
            # print("Ping")
            ticker_ping()
        elif words[0] == "StartCountdown":
            # print("StartCountdown")
            if (len(words)>3):
                ticker_StartCountdown(int(words[1]),int(words[2]),int(words[3]))
        elif words[0] == "ResetCountdown":
            # print("ResetCountdown")
            if (len(words)>3):
                ticker_ResetCountdown(int(words[1]),int(words[2]),int(words[3]))

def show_time():
    global deltaTime
    global endTime
    global currentdisplaycolour
    global currentdisplaytime
    
    # debug_print("showtime");
    if (clock_mode == 1):
        # Get the time remaining until the event
        deltaTime = endTime - datetime.datetime.now()
    
    if (clock_mode == 2):
        # Get the time remaining until the event
        endTime = datetime.datetime.now() + deltaTime

    if (deltaTime.total_seconds()<(-10*60*60+1)):
        ticker_updateStatus("",3)

    # remove the microseconds part
    ms = deltaTime.microseconds
    deltaTime = deltaTime - datetime.timedelta(microseconds=deltaTime.microseconds)
    # print(pretty_time_delta(deltaTime.total_seconds()))
    if deltaTime.total_seconds()>2*60:
        colour = GREEN
    else:
        colour = RED
    if deltaTime.total_seconds()<0:
        if ms>0 and ms<300000:
            colour = BLACK

    if (clock_mode == 3):
        colour = BLACK
        if (currentdisplaycolour!=colour):
            screen.fill(BLACK)
            pygame.display.update()
            currentdisplaycolour=colour
            currentdisplaytime=""
    else:
        # Show the time left
        timestr = pretty_time_delta(deltaTime.total_seconds())
        ticker_updateStatus(timestr,clock_mode)
        if ((currentdisplaycolour!=colour) | (currentdisplaytime!=timestr)):
            screen.fill(BLACK)
            debug_print("font render")
            text = font.render(timestr, True, (colour))
            x = (screen_width/2 - (text.get_width()) // 2)
            y = (screen_height/2 - (text.get_height()) // 2)
            debug_print("screen blit")
            screen.blit (text, (x,y))
            pygame.display.update()
            currentdisplaycolour=colour
            currentdisplaytime=timestr
        
    #process content on serial port
    moreBytes = ser.inWaiting()
    if moreBytes:
        processSerial()

        
# Main program
if (platform.system()=="Windows"):
    ser = serial.Serial("COM4",115200)
    pygame.time.delay(1000)
else:
    ser = serial.Serial("/dev/ttyUSB0",115200)

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
debug_print("pygame init")
pygame.init()
debug_print("pygame set_mode")
screen = pygame.display.set_mode((0,0),pygame.FULLSCREEN)
# screen = pygame.display.set_mode((0,0))
surface = pygame.display.get_surface() 
screen_width,screen_height = size = surface.get_width(), surface.get_height()
pygame.mouse.set_visible(0)

go = True

#Define Colours
GREEN = (0,255,0)
RED = (255,0,0)
BLACK = (0,0,0)

currentdisplaycolour=BLACK
currentdisplaytime=""

if (platform.system()=="Windows"):
    font_size = int(screen_width/4.25)
else:
    font_size = int(screen_width/2.9)
font = pygame.font.SysFont ("Helvetica", font_size, True)

ticker_updateStatus("",3)

while go:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            go = False

        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_x:
                ticker_quit()
                
            elif event.key == pygame.K_r:
                resetticker()

            elif event.key == pygame.K_e:
                resettickerpause()

            elif event.key == pygame.K_s:
                ticker_stop()

            elif event.key == pygame.K_p:
                ticker_pause()

            elif event.key == pygame.K_t:
                ticker_test()

    pygame.time.delay(100)
    show_time()
