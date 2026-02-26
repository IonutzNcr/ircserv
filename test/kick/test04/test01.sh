#!/usr/bin/expect -f
set timeout 10

set NICK "boy1"
set SERVER "localhost"
set PORT 4444
set HOST "host"
set PSD "lol"

spawn irssi -c $SERVER -w $PSD -p $PORT -n $NICK -h $HOST
sleep 1

send "/join #monchan\r"
send "/join #monchan2\r"
sleep 5
send "/kick #monchan,#monchan2 girl,girl1 tg\r"

interact