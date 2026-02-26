#!/usr/bin/expect -f
set timeout 10

set NICK "girl1"
set SERVER "localhost"
set PORT 4444
set HOST "host3"
set PSD "lol"

spawn irssi -c $SERVER -w $PSD -p $PORT -n $NICK -h $HOST

send "/join #monchan\r"
send "/join #monchan2\r"

interact