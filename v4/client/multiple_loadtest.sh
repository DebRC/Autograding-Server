#!/bin/bash

# Checking for correct arguments
if [ $# -ne 3 ]; then
    echo "usage: ./multiple_loadtest.sh <IP:Port> <testFile> <n>"
    exit 1
fi

mkdir -p client_logs

server_address=$1
file_to_grade=$2
n=$3

total_time=0

# Run load tests in the background
for ((i=1; i<=$n; i++)); do
    bash single_loadtest.sh "$server_address" "$file_to_grade" > "client_logs/time_taken_$i.txt" &
done

# Wait for all background processes to finish
wait

# Calculate the total time taken
for ((i=1; i<=$n; i++)); do
    time_taken=$(cat "client_logs/time_taken_$i.txt" | grep -o 'Time taken for grading: [0-9]*' | awk '{print $NF}')
    # echo $time_taken
    total_time=$((total_time + time_taken))
done

echo "Total time taken for $n load tests: $total_time seconds"


killChildren() {
    pkill -P $$  # Send the termination signal to all child processes
    exit 0  # Exit the script
}

trap killChildren SIGINT SIGTERM
