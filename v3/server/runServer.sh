#!/bin/bash
# check if the inputs are correct or not
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <NumberOfCores> <PortNum> <ThreadPoolSize> <RequestQueueSize>"
    exit 1
fi

# fetch the inputs
core=$1
port=$2
threadPoolSize=$3
requestQueueSize=$4

# creating directories
mkdir -p logs
mkdir -p logs/files
mkdir -p logs/outputs

# run the server
gcc -o server server.c utils/helper.c utils/circular_queue.c utils/make_filename.c utils/system_commands.c utils/file_transfer.c

#handling the core
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
if [[ $core -eq 5 ]]; then
    taskset -c 0,1,2,3,4 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 6 ]]; then
    taskset -c 0,1,2,3,4,5 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 7 ]]; then
    taskset -c 0,1,2,3,4,5,6 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 8 ]]; then
    taskset -c 0,1,2,3,4,5,6,7 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 9 ]]; then
    taskset -c 0,1,2,3,4,5,6,7,8 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 10 ]]; then
    taskset -c 0,1,2,3,4,5,6,7,8,9 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 11 ]]; then
    taskset -c 0,1,2,3,4,5,6,7,8,9,10 ./server $port $threadPoolSize $requestQueueSize
fi
if [[ $core -eq 12 ]]; then
    taskset -c 0,1,2,3,4,5,6,7,8,9,10,11 ./server $port $threadPoolSize $requestQueueSize
fi

# kill all the child proceess
killChildren() {
    pkill -P $$  # Send the termination signal to all child processes
    exit 0  # Exit the script
}

# kill all the child processes using trap
trap killChildren SIGINT SIGTERM

# run the utilization script in the background
bash utilization_script.sh $port &

# run the server with port
./server $port
