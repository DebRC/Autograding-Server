#!bin/bash

# Checcking for correct arguments
if [ $# -ne 5 ];then
    echo "usage: ./client_run.sh <IP:Port> <testFile> <numClients> <numOfLoop> <sleepTime>"
fi

# initializing the variables with command line inputs
ip_port=$1
test_file=$2
num_of_client=$3
num_of_loop=$4
sleep_time=$5

# compile the client code to get the executable
gcc -o client gradingclient.c

# looping to run the multiple clients parallel
for ((num=1; num<=$num_of_client; num++)); do
    bash loadtest.sh $ip_port $test_file $num $num_of_loop $sleep_time
done

# plotting the graphs
python3 plot.py output.csv
