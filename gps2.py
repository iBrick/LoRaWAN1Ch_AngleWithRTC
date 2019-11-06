#! /usr/bin/python
import os
import sys
from gps import *
from time import *
import time
import threading
import json
import paho.mqtt.client as paho
import socket
import signal

gpsd = None

class GpsPoller(threading.Thread):
  def __init__(self):
    threading.Thread.__init__(self)
    global gpsd #bring it in scope
    gpsd = gps(mode=1) #starting the stream of info
    self.current_value = None
    self.running = True #setting the thread running to true

  def run(self):
    global gpsd
    while gpsp.running:
      gpsd.next() #this will continue to loop and grab EACH set of gpsd info to clear the buffer

def exit_gracefully(signum, frame):
  print("Exited")

def onConnect(client, userdata, flags, rc):
  print("Connected With Result Code " (rc))

def onDisconnect(client, userdata, rc):
  print("Client Got Disconnected")

def restartlily():
  print(sys.executable, ['python3'] + sys.argv)
  os.execv(sys.executable, ['python3'] + sys.argv)
  return

if __name__ == '__main__':
  gpsp = GpsPoller() # create the thread
  print("GPS script started")
  try:
    gpsp.start() # start it up
    client = paho.Client()
    client.on_connect = onConnect
    client.on_disconnect = onDisconnect
    client.username_pw_set("dfffsfdf","34343434")
    try:
      client.connect("91.77.164.13", 1883, 60)
    except socket.gaierror as e:
      print('Gaierror {}.'.format(e))
      signal.signal(signal.SIGTERM, exit_gracefully)
      time.sleep(10)
      client.connect("91.77.164.13", 1883, 60)

    client.loop_start()
    print("Before loop")

    while True:
      if gpsd.fix.latitude > 0:
        row = [ { 'latitude': str(gpsd.fix.latitude),
         'longitude': str(gpsd.fix.longitude),
         'utc': str(gpsd.utc),
         'time':   str(gpsd.fix.time),
         'altitude': str(gpsd.fix.altitude),
         'satellites': str(gpsd.satellites),
         'hdop': str(gpsd.hdop),
         'epv': str(gpsd.fix.epv),
         'ept': str(gpsd.fix.ept),
         'speed': str(gpsd.fix.speed),
         'climb': str(gpsd.fix.climb),
         'track': str(gpsd.fix.track),
         'mode': str(gpsd.fix.mode)} ]

        json_string = json.dumps(row)

        client.publish("app/gps/device/000000ffff000000/rx", payload=json_string, qos=1, retain=True)
        time.sleep(60)
	print("working")
  #except (socket.error):
  #  print("Exit on socket error")
  #  exit(1)
  #  client.disconnect() # disconnect
  #  quit()
  #  raise Exception('Stop program')
  except (KeyboardInterrupt): #when you press ctrl+c
    gpsp.running = False
    gpsp.join() # wait for the thread to finish what it's doing
