#!/bin/bash

# checking for the correct inputs
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <PortNum>"
    exit 1
fi

# taking the port number
port=$1

# creating the necessary folders
mkdir -p logs
mkdir -p logs/files
mkdir -p logs/outputs

# compiling the server
gcc -o server server.c

killChildren() {
    pkill -P $$  # Send the termination signal to all child processes
    exit 0  # Exit the script
}

# Terminating all the child process
trap killChildren SIGINT SIGTERM

# run the utilization script
./utilization_script.sh $port &

# run the server program
./server $port
