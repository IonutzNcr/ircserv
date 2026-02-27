#!/usr/bin/expect -f
set timeout 10

set NICK "girl"
set SERVER "localhost"
set PORT 4444
set HOST "host4"
set PSD "lol"

spawn irssi -c $SERVER -w $PSD -p $PORT -n $NICK -h $HOST


send "/join #monchan ptr\r"
send "/join #kickchan ptr\r"

interact