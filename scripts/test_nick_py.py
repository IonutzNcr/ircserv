#!/usr/bin/env python3
"""
Minimal Python test to send PASS, NICK, USER to the IRC server and print server replies.
Usage:
    python3 scripts/test_nick_py.py
"""
import socket
import time

HOST = 'localhost'
PORT = 4444

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.connect((HOST, PORT))
except Exception as e:
    print('Connect failed:', e)
    raise

def send(line):
    print('->', line)
    s.sendall((line + '\r\n').encode())
    time.sleep(0.2)

send('PASS caca')
send('NICK toto')
send('USER arnaud 0 * :Arnaud')

# wait for responses
time.sleep(1)

s.settimeout(2.0)
recv = b''
try:
    while True:
        chunk = s.recv(4096)
        if not chunk:
            break
        recv += chunk
except socket.timeout:
    pass

print('\n--- SERVER REPLY ---')
print(recv.decode(errors='replace'))
print('--- END ---')

s.close()