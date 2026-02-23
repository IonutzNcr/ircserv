#!/usr/bin/expect -f
set timeout 10

set NICK "boy"
set SERVER "localhost"
set PORT 4444
set HOST host2
set PSD "lol"

spawn irssi -c $SERVER -w $PSD -p $PORT -n $NICK -h $HOST
sleep 1

send "/join #monchan ptr\r"
send "/join #kickchan ptr\r"

sleep 4

send "/kick #monchan,#kickchan troll,girl\r"
interact