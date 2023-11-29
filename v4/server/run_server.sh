#!/bin/bash
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <NumberOfCores> <PortNum> <ThreadPoolSize>"
    exit 1
fi

core=$1
port=$2
threadPoolSize=$3

echo "Creating Required Directories ... "
mkdir -p logs
mkdir -p logs/files
mkdir -p logs/outputs

echo "Creating Required Files ... "
# Check if file exists
if [ ! -e "request_status.csv" ]; then
    touch "$filename"
fi

echo "Running Make File ... "
make

echo "Done"

echo "Running Server ... "

if [[ $core -gt 0 && $core -le $(nproc) ]]; then
    taskset_str="taskset -c $(seq -s, 0 $((core-1)))"
    $taskset_str ./server $port $threadPoolSize
else
    echo "Invalid Core Count. It should be between 1 and $(nproc)."
fi

killChildren() {
    pkill -P $$  # Send the termination signal to all child processes
    exit 0  # Exit the script
}

trap killChildren SIGINT SIGTERM

# ./utilization_script.sh $port &