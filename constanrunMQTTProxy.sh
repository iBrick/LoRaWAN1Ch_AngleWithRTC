#!/bin/sh
sleep 10
COMMAND='/usr/bin/python /home/pi/PyMQTTProxy.py'
LOGFILE=restartMQTTProxy.txt

writelog() {
  now=`date`
  echo "$now $*" >> $LOGFILE
}

writelog "Starting"
while true ; do
  $COMMAND
  writelog "Exited with status $?"
  writelog "Restarting"

done
