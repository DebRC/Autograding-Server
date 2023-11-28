#!/bin/bash
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <NumberOfCores> <PortNum> <ThreadPoolSize>"
    exit 1
fi

core=$1
port=$2
threadPoolSize=$3

mkdir -p logs
mkdir -p logs/files
mkdir -p logs/outputs

> "request_status.csv"

make clean

make

if [[ $core -eq 1 ]]; then
    taskset -c 0 ./server $port $threadPoolSize
fi
if [[ $core -eq 2 ]]; then
    taskset -c 0,1 ./server $port $threadPoolSize
fi
if [[ $core -eq 3 ]]; then
    taskset -c 0,1,2 ./server $port $threadPoolSize
fi
if [[ $core -eq 4 ]]; then
    taskset -c 0,1,2,3 ./server $port $threadPoolSize
fi

killChildren() {
    pkill -P $$  # Send the termination signal to all child processes
    exit 0  # Exit the script
}

trap killChildren SIGINT SIGTERM

# ./utilization_script.sh $port &

./server $port
