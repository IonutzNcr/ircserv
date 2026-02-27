*This project has been created as part of the 42 curriculum by radlouni, inicoara, amugisha.*

# IRC Server – 42 Project

## Description

This project consists of building a minimal IRC (Internet Relay Chat) server in C++98, compliant with the IRC protocol (RFC 1459).

The goal is to understand network programming, socket management, and concurrent client handling while respecting the constraints of the C++98 standard.

The server allows multiple clients to connect simultaneously, register with a nickname and username, join channels, and communicate with each other using basic IRC commands.

This project was developed as part of the 42 Paris curriculum.

---

## Features

- TCP socket server
- Multiple clients management (poll/select)
- Authentication with password
- Nickname and username registration
- Channel creation and management
- Private messages
- Basic IRC commands:
  - PASS
  - NICK
  - USER
  - JOIN
  - PART
  - PRIVMSG
  - QUIT
  - TOPIC
  - KICK
  - INVITE
  - MODE +otilk

---

## Instructions

### Requirements

- C++98
- Unix-based system (Linux / macOS)
- Make

### Install

```
git clone repo name_repo

cd name_repo
```

Build Porject
```
make
```

Run server exemple
```
./ircserv port(4444) password(lol)
```

Run client exemple
```
irssi --connect=localhost --password=lol --port=4444 -n arnaud -h localhost
```

### Compilation

```bash
make (for build server)
make clean (for clean .o)
make fclean (for remove server and .o)
make re (for make fclean and make)
```
## Resources

- https://man7.org/linux/man-pages/man5/elf.5.html
