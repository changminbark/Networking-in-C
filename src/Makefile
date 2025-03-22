# https://chatgpt.com/share/67d7221b-4030-8011-b152-1fc85020c07e
# Compiler and Flags
CC = gcc
CFLAGS = -std=gnu11 -pedantic -Wall -g -Werror -Wextra

# Executable Names
EXECS = demo_server demo_server_err sclient sserver

# All Executables
all: $(EXECS)

# Debug Mode (Appends -DDEBUG to CFLAGS)
debug: CFLAGS += -DDEBUG
debug: clean all

# Compilation Rules for Executables
sclient: sclient.c spotify.o
	$(CC) $(CFLAGS) sclient.c spotify.o -o sclient

demo_server: demo_server.c spotify.o
	$(CC) $(CFLAGS) demo_server.c spotify.o -o demo_server

demo_server_err: demo_server_err.c spotify.o
	$(CC) $(CFLAGS) demo_server_err.c spotify.o -o demo_server_err

# Compilation Rule for sserver
sserver: sserver.o spotify.o sbrowser.o htable.o slist.o snode.o
	$(CC) $(CFLAGS) $^ -o sserver

# Compilation Rules for Object Files
sserver.o: sserver.c sbrowser.h spotify.h htable.h slist.h snode.h
	$(CC) $(CFLAGS) -c $< -o $@

sbrowser.o: sbrowser.c sbrowser.h spotify.h htable.h slist.h snode.h
	$(CC) $(CFLAGS) -c sbrowser.c -o sbrowser.o

spotify.o: spotify.c spotify.h 
	$(CC) $(CFLAGS) -c spotify.c -o spotify.o

htable.o: htable.c htable.h slist.h snode.h
	$(CC) $(CFLAGS) -c $< -o htable.o

slist.o: slist.c slist.h snode.h
	$(CC) $(CFLAGS) -c slist.c -o slist.o

snode.o: snode.c snode.h
	$(CC) $(CFLAGS) -c snode.c -o snode.o

# Clean Rule (Remove Binaries & Object Files)
.PHONY: clean
clean:
	/bin/rm -rf $(EXECS) *.o *~
