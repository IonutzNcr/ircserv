#!/usr/bin/expect -f
set timeout 10

set NICK "boy"
set SERVER "localhost"
set PORT 4444
set HOST host2
set PSD "lol"

spawn irssi -c $SERVER -w $PSD -p $PORT -n $NICK -h $HOST
sleep 4

send "/join #monchan\r"
send "/join #monchan2\r"
sleep 3
send "/join #prout null,ptr\r"
interact