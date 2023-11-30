#!/bin/bash

# Checcking for correct arguments
if [ $# -ne 6 ];then
    echo "usage: ./client_run.sh <IP:Port> <testFile> <numClients> <numOfLoop> <sleepTime> <time-out>"
    exit 1
fi

# initalize all the variables with the command line inputs
ip_port=$1
test_file=$2
num_of_client=$3
num_of_loop=$4
sleep_time=$5
time_out=$6

# execute the loadtest 
for ((num=1; num<=$num_of_client; num++)); do
    bash loadtest.sh $ip_port $test_file $num $num_of_loop $sleep_time $time_out
done

# plot the graph
python3 plot.py output.csv
