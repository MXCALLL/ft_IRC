*This project has been created as part of the 42 curriculum by muidbell, obensarj, hbenmoha.*

# ft_irc; Internet Relay Chat Server

## Description

ft_irc is a fully functional IRC server written in C++98, built as part of the 42 school curriculum. The goal of the project is to implement the core IRC protocol from scratch, allowing real IRC clients to connect and communicate through it.

The server handles multiple simultaneous client connections using a single non-blocking `poll()` event loop, no threads, no forking. It follows the IRC protocol as defined in RFC 1459, supporting authentication, channel management, private messaging, and operator commands.

The project is divided across three areas of responsibility:

- **muidbell** : server architecture, socket setup, non-blocking I/O, client registration (PASS, NICK, USER), bot (bonus)
- **hbenmoha** : channel commands: JOIN, INVITE, KICK, file transfer (bonus)
- **obensarj** : operator commands: MODE, TOPIC, PRIVMSG

## Features

The server supports the following:

- Multi-client handling via `poll()` with non-blocking file descriptors
- Full client registration flow: PASS, NICK, USER
- Channel creation and management
- Public and private messaging via PRIVMSG
- Channel operator commands: KICK, INVITE, TOPIC, MODE
- MODE flags: `+i` (invite-only), `+t` (topic restriction), `+k` (channel password), `+l` (user limit), `+o` (operator privilege)
- Graceful shutdown on SIGINT and SIGQUIT
- Automatic cleanup of empty channels on disconnect

## Instructions

### Compilation

```bash
make
```

This produces the executable `ircserv`. To clean build artifacts:

```bash
make clean    # removes object files
make fclean   # removes object files and binary
make re       # full recompile
```

### Running the Server

```bash
./ircserv <port> <password>
```

- `port` must be between 1024 and 65535
- `password` cannot be empty

Example:

```bash
./ircserv 6667 mypassword
```

### Connecting with an IRC Client

The reference client used for this project is **irssi**.

#### What is irssi?

irssi is a modular, terminal-based Internet Relay Chat client. It is highly extensible and widely used for IRC communication.

```bash
# Install irssi (if not already installed)
brew install irssi        # macOS
sudo apt install irssi    # Ubuntu/Debian
```

#### Basic Usage

```bash
irssi [--config=PATH] [--home=PATH] [-c server] [-p port] [-n nickname] [-w password]

# For full options:
man irssi
```

#### Connecting to the Server

**Option 1 — Launch with nickname:**
```bash
irssi -n yournickname
```

**Option 2 — Launch without nickname:**
```bash
irssi
# irssi will use the nickname defined in ~/.irssi/config
```

Then inside irssi, connect to the server:
```bash
/connect 127.0.0.1 6667 mypassword

Or with a nickname directly :

/connect 127.0.0.1 6667 mypassword yournickname
```

### Testing with netcat

For raw protocol testing:

```bash
nc -C 127.0.0.1 6667
```

Then type IRC commands manually:

```
PASS mypassword
NICK testuser
USER testuser 0 * :Test User
JOIN #general
```

## Resources

### IRC Protocol References

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [Alpha Guide to Creating an IRC Server from Scratch (Video Playlist)](https://youtube.com/playlist?list=PLHBVNH27RbWqGTL-AYMylWkNck45cxPnG&si=SdYgA4SCvRADjbMl)
- [IRC Numeric Replies Reference](https://www.alien.net.au/irc/irc2numerics.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [Socket Programming in C++](https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/)

### AI Usage

All three team members used **Claude (Anthropic)** as an AI assistant throughout the project.

Claude was used interactively as a learning and debugging tool — explaining socket programming and networking concepts, clarifying RFC 1459 protocol requirements and message formats, and guiding the implementation of each command. It was also used to understand C++ data structures (maps, iterators, references) in the context of the project, and to design and run edge case tests using `nc` and custom Python scripts.

In all cases, Claude guided the team toward solutions rather than generating complete code directly, the understanding and implementation decisions remained with the developers.
