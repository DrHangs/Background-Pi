#!/usr/bin/env python3

import RPi.GPIO as GPIO
import time, datetime, os

with open("config") as f:
        conf_raw = f.read()
conf_lines = conf_raw.split('\n')
for line in conf_lines:
        if(line.startswith('#')):
                continue
        if(line.split('=')[0] == "pin-conf"):
                conf_pin = line.split('=')[1]

GPIO.setmode(GPIO.BCM)
GPIO.setup(conf_pin, GPIO.IN, GPIO.PUD_UP)

while True:
        while(GPIO.input(conf_pin) == 1):
                time.sleep(0.1)
        while(GPIO.input(conf_pin) == 0):
                time.sleep(0.01)
        with open('history.log', mode='w') as f:
                f.write(f"Started at {datetime.datetime.now()}" + os.linesep)
        os.system('./bkglight')
