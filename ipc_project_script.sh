#!/bin/bash
clear
rm *.o
rm server client_1 client_2 client_3

gcc server.c -o server -lpthread
gcc -c client*.c
gcc -c *library.c

gcc client_1.o Async_library.o Sync_library.o -o client_1 -lpthread
#gcc client_1a.o Async_library.o Sync_library.o -o client_1a -lpthread
gcc client_2.o Async_library.o Sync_library.o -o client_2 -lpthread
#gcc client_2a.o Async_library.o Sync_library.o -o client_2a -lpthread
gcc client_3.o Async_library.o Sync_library.o -o client_3 -lpthread
#gcc client_3a.o Async_library.o Sync_library.o -o client_3a -lpthread

#./server &
gnome-terminal --working-directory=/home/nitesh/Desktop/ipc -e './client_1 -async'
gnome-terminal --working-directory=/home/nitesh/Desktop/ipc -e './client_1 -sync'
gnome-terminal --working-directory=/home/nitesh/Desktop/ipc -e './client_2 -async'
gnome-terminal --working-directory=/home/nitesh/Desktop/ipc -e './client_2 -sync'
gnome-terminal --working-directory=/home/nitesh/Desktop/ipc -e './client_3 -async'
gnome-terminal --working-directory=/home/nitesh/Desktop/ipc -e './client_3 -sync'
rm *.o

