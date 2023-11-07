#!/bin/bash
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <NumberOfCores> <PortNum> <ThreadPoolSize> <RequestQueueSize>"
    exit 1
fi

core=$1
port=$2
threadPoolSize=$3
requestQueueSize=$4

mkdir -p logs
mkdir -p logs/files
mkdir -p logs/outputs

gcc -o server server.c circular_queue.c

if [[ $core -eq 1 ]]; then
    taskset -c 0 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 2 ]]; then
    taskset -c 0,1 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 3 ]]; then
    taskset -c 0,1,2 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 4 ]]; then
    taskset -c 0,1,2,3 ./server $port $threadPoolSize $requestQueueSize
fi

killChildren() {
    pkill -P $$  # Send the termination signal to all child processes
    exit 0  # Exit the script
}

trap killChildren SIGINT SIGTERM

./utilization_script.sh $port &

./server $port
