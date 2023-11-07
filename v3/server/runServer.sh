#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <PortNum>"
    exit 1
fi

port=$1

mkdir -p logs
mkdir -p logs/files
mkdir -p logs/outputs

gcc -o server server.c

killChildren() {
    pkill -P $$  # Send the termination signal to all child processes
    exit 0  # Exit the script
}

trap killChildren SIGINT SIGTERM

./utilization_script.sh $port &

./server $port
