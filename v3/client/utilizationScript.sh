#!/bin/bash

# taking the port no from input
port_no=$1

# Extracting the cpu utilization and thread count
while true
do
    vmstat 1 2 | tail -1 | awk '{print $13}' >> cpu_utilization_snapshots.txt
    ps -eLf | grep "./server $port_no" | head -1 | awk '{print $6}' >> thread_count_snapshots.txt
    sleep 5
done
