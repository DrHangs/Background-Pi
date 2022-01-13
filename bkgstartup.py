#!/usr/bin/env python3

import RPi.GPIO as GPIO
import time, datetime, os, psutil

def findProcessIdByName(processName):
    '''
    Get a list of all the PIDs of a all the running process whose name contains
    the given string processName
    '''
    listOfProcessObjects = []
    #Iterate over the all the running process
    for proc in psutil.process_iter():
       try:
           pinfo = proc.as_dict(attrs=['pid', 'name', 'create_time'])
           # Check if process name contains the given name string.
           if processName.lower() in pinfo['name'].lower() :
               listOfProcessObjects.append(pinfo)
       except (psutil.NoSuchProcess, psutil.AccessDenied , psutil.ZombieProcess) :
           pass
    return listOfProcessObjects

with open("config") as f:
        conf_raw = f.read()
conf_lines = conf_raw.split('\n')
for line in conf_lines:
        if(line.startswith('#')):
                continue
        if(line.split('=')[0] == "pin-conf"):
                conf_pin = line.split('=')[1]
conf_pin = 17 # Unterschiedliche Nummerierung bei BCM und WiringPi
GPIO.setmode(GPIO.BCM)
GPIO.setup(conf_pin, GPIO.IN, GPIO.PUD_UP)
if(len(findProcessIdByName("pi-blaster")) == 0):
        # Standard-Pins von pi-blaster ändern
        os.system("pi-blaster --gpio 13,19,26")

while True:
        while(GPIO.input(conf_pin) == 1):
                time.sleep(0.1)
        while(GPIO.input(conf_pin) == 0):
                time.sleep(0.01)
        with open('history.log', mode='a') as f:
                f.write(f"Started at {datetime.datetime.now()}" + os.linesep)
        os.system('./bkglight')
