# Chat server using C lang

```
This project is a simple multithreaded chat server written in C, utilizing socket programming and select() to handle multiple clients simultaneously. Each client can send and receive messages in real-time, with the chat history being saved to a chat-log file.
```

## Features
- Multithreaded Server: Supports multiple clients connecting and chatting simultaneously.
- Real-time Messaging: Messages are broadcast to all connected clients, except the sender (Avoid duplicated messaging on sender screen).
- Chat History: All messages are saved to a chat_log.txt file.
- User Identification: Messages are tagged with the sender's name.

## Prerequisites
- GCC compiler
- Linux / UNIX 

Installation
Clone the repository:
```sh
git clone https://github.com/SPC-Toad/c-chat-server.git
cd c-chat-server
```
Compile the server and client:
```sh
make server
make client
```

Start the server:
```sh
# Replace <port> with the desired port number and <max_clients> with the maximum number of clients allowed.
./server <port> <max_clients>

# or simply run:
make run-server
# which will run the server at port 7777
```

Start a client:
```sh
# Replace <port> with the port number the server is listening on, and <your_name> with your desired display name.
./client <port> <your_name>
```

## Note
Clients can type messages which will be broadcast to all other connected clients.
The chat history is saved in chat_log.txt.

## Example Run:

## Thank you for stopping by!