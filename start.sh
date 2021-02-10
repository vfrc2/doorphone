#!/bin/bash
 DOORPHONE_PID_FILE=./doorphone.pid
export DOORPHONE_PID_FILE

./doorphone -d sip:192.168.88.26:5061 &

echo Start gpio mon
until [ 1 = 0 ]
do
    gpiomon -fs -n 1 0 4
    echo Presed
    kill -s USR1 $(cat $DOORPHONE_PID_FILE)
    sleep 1s
done