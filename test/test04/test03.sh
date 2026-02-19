#!/usr/bin/expect -f
set timeout 10

set NICK "troll"
set SERVER "localhost"
set PORT 4444
set HOST "host3"
set PSD "lol"

spawn irssi -c $SERVER -w $PSD -p $PORT -n $NICK -h $HOST
sleep 3

send "/join #monchan,#chan2,#chan3\r"

interact