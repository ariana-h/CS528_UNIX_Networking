# Morse Code UDP Client-Server


## Overview
This project implements a simple UDP-based client-server system that translates plaintext messages into Morse code. The server receives plaintext messages from clients and responds with the corresponding Morse code representation. Each Morse code symbol is sent as a separate character, with letters separated by `#` and words separated by spaces. The client can optionally decode the Morse code back into plaintext.

## Features
- **UDP Communication**: The client and server communicate using the UDP protocol.
- **Morse Code Encoding**: The server translates received text into Morse code.
- **Morse Code Decoding**: The client can decode the Morse response if the `-d` flag is specified.
- **Packet Drop Handling**: In case of packet loss, undecodable letters are replaced with `?`.

## File Structure
- `client.c` - The UDP client that sends plaintext messages and optionally decodes Morse responses.
- `server.c` - The UDP server that translates messages into Morse code and sends them back to the client.

## Compilation
To compile both the client and server:
```sh
gcc -o client client.c
gcc -o server server.c
```

## Usage
### Running the Server
```sh
./server <SERVER_IP>
```
Example:
```sh
./server 127.0.0.1
```

### Running the Client
```sh
./client <SERVER_IP> [-d]
```
Example:
```sh
./client 127.0.0.1
```
To enable Morse code decoding:
```sh
./client 127.0.0.1 -d
```

### Client Interaction
- The client prompts the user to enter a message.
- The message is sent to the server.
- The server responds with Morse code.
- If `-d` is specified, the client decodes the Morse code and prints the translated text.
- Type `exit` to terminate the client.

## Morse Code Format
- Each Morse symbol (`o` for dot, `-` for dash) is sent as an individual character.
- Letters are separated by `#`.
- Words are separated by spaces.

## Why UDP?
UDP is used instead of TCP for speed and efficiency. Since Morse code can tolerate minor packet loss without significantly affecting understanding, UDP allows faster transmission while introducing an interesting challenge related to real-world packet loss scenarios.
