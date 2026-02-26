#!/usr/bin/expect -f
set timeout 10

set NICK "boy"
set SERVER "localhost"
set PORT 4444
set HOST host2
set PSD "lol"

spawn irssi -c $SERVER -w $PSD -p $PORT -n $NICK -h $HOST
sleep 1

send "/join #monchan\r"
sleep 5
send "/kick #monchan girl,girl1,girl2 ca pue la merde\r"

interact