#!/bin/bash

# taking the port from the input
port=$1

# storing the utilization snapshots
snapshotFile="logs/utilization_snapshot.csv"
> $snapshotFile

# initialize the variables
total_nlwp=0
total_cpu=0
count=0

while true; do
    # Get the NLWP of the "server" process
    nlwp=$(ps -T -C "server" -o pid,ppid,nlwp,cmd | grep "./server $port" | awk '{print $3}' | head -1)
    echo "Threads :: $nlwp" >> $snapshotFile
    
    # Get the CPU Utilization
    cpu_util=$(vmstat 1 2 | tail -n 1 | awk '{print $13}')
    echo "CPU Utilization :: $cpu_util" >> $snapshotFile

    # Add the NLWP and CPU utilization to the running totals
    total_nlwp=$((total_nlwp + nlwp))
    total_cpu=$(bc -l <<< "$total_cpu + $cpu_util")
    
    # Increment the count
    count=$((count + 1))
    
    echo "" >> $snapshotFile

    # Calculate and write the average NLWP and CPU utilization till now
    if [ $count -gt 0 ]; then
        avg_nlwp=$(bc -l <<< "scale=4; $total_nlwp / $count")
        avg_cpu=$(bc -l <<< "scale=4; $total_cpu / $count")
        echo "Average Threads (till now) :: $avg_nlwp" >> $snapshotFile
        echo "Average CPU Utilization (till now) :: $avg_cpu" >> $snapshotFile
    fi

    echo "" >> $snapshotFile
    sleep 2
done