# C Networking App - Spotify Song Server

This is a C-based networking application that simulates a basic Spotify-like server-client system, along with a web browser-like interface for song search and retrieval. It demonstrates concepts like socket programming, hash tables, linked lists, and error handling in C.

## Features

- Client-server architecture using sockets
- Custom browser interface to interact with the server
- Song search functionality via linked list and hash table
- Server-side error handling
- CSV-based song data storage (`spotify_songs.csv`)

## File Overview

### Networking
- `demo_server.c` / `demo_server_err.c` - Example servers showcasing socket usage with and without error handling
- `sclient.c` - Socket client implementation
- `sserver.c` - Custom Spotify song server using socket communication

### Web-like Browser
- `sbrowser.c` - Command-line interface simulating browser functionality to connect with the song server
- `sbrowser.h` - Header file for the browser module

### Data Structures
- `htable.c` / `htable.h` - Hash table implementation for song indexing
- `slist.c` / `slist.h` - Singly linked list implementation for managing song entries
- `snode.c` / `snode.h` - Node definition for linked list elements

### Spotify-Specific
- `spotify.c` / `spotify.h` - Core logic for handling Spotify song metadata
- `spotify_songs.csv` - Sample song dataset used for indexing and searching

### Build
- `Makefile` - Automates compilation of the server, client, and browser components

## Usage

### Build the project
```bash
make
```

### Run the server

Start the main song server, which reads song data from `spotify_songs.csv` and listens for client connections.

```bash
./sserver
```

### Run the client browser

Launch the command-line browser interface to connect to the Spotify song server. This client allows you to search for songs and retrieve information from the server.

```bash
./sbrowser
```

### Cleanup

```bash
make clean
```

## Requirements

To run this project, you will need:

- A C compiler (e.g., `gcc`)
- A Unix-like operating system (Linux or macOS recommended)
- A terminal or shell environment to compile and run the programs
- Basic understanding of socket programming (helpful for customization)

No external libraries are required—this project is built entirely with standard C and POSIX sockets.

