#!/usr/bin/expect -f
set timeout 10

set NICK   "girl"
set SERVER "localhost"
set PORT   4444
set HOST   "host1"
set PSD    "lol"

spawn irssi -c $SERVER -w $PSD -p $PORT -n $NICK -h $HOST
sleep 1

send "/join #monchan\r"
sleep 10
send "/kick #monchan boy,troll :ta mere pue\r"
interact
