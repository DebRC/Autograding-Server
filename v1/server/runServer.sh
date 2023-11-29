#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <PortNum>"
    exit 1
fi

# taking the port value
port=$1

# creating the necessary folders
mkdir -p logs
mkdir -p logs/files
mkdir -p logs/outputs

# compile the server program
gcc -o server server.c

# execute the server with the given port
./server $port
