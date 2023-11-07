#!bin/bash

# Checcking for correct arguments
if [ $# -ne 5 ];then
    echo "usage: ./loadtest.sh <IP:Port> <testFile> <numClients>  <loopNum>  <sleepTimeSeconds>"
    exit 1
fi

# Variables used
ipPort=$1
testFile=$2
numOfClient=$3
numOfLoop=$4
sleepTime=$5

mkdir -p client_logs
mkdir -p client_logs/$numOfClient

totalResponseTime=0.000000
overall_throughput=0.000000
overall_response_time=0.000000
total_response=0

# Executing the client
for ((num=1; num<=$numOfClient; num++)); do
    output_file="output_$num.txt"
    ./client "$ipPort" "$testFile" "$numOfLoop" "$sleepTime" > "./client_logs/$numOfClient/$output_file" &
    pids[num]=$!
done

# Wait until all the processes finishes
for ((i=1; i<=$numOfClient; i++)); do
    wait "${pids[$i]}"
done



# CALCULATING THE AVERAGE RESPONSE TIME
for ((num=1; num<=$numOfClient; num++)); do
    # File name in the current directory
    file_name="./client_logs/$numOfClient/output_$num.txt"

    # Use grep and awk to search and extract the float value
    if grep -q "Average response time" "$file_name"; then
        client_avg_response_time=$(grep "Average response time" "$file_name" | awk '{print $4}')
    fi
    if grep -q "The number of Successful response" "$file_name"; then
        num_of_response=$(grep "The number of Successful response" "$file_name" | awk '{print $6}')
        # echo "Found no of response: $num_of_response"
    fi
    total_response=$(echo "scale=6; $total_response + $num_of_response" | bc -l)
    # echo "total successful response: $total_response"
    client_total_response_time=$(echo "scale=6; $client_avg_response_time * $num_of_response" | bc -l)
    # echo "client total response time: $client_total_response_time"
    overall_response_time=$(echo "scale=6; $overall_response_time + $client_total_response_time" | bc -l)
    # echo "client overall response time: $overall_response_time"
done
# echo "number of client: $numOfClient"
total_response=${total_response}.000000
avgResponseTime=$(echo "scale=6; $overall_response_time / $total_response" | bc -l)
echo "Average response time: $avgResponseTime"



# CALCULATING THE OVERALL THROUGHPUT
for ((num=1; num<=$numOfClient; num++)); do
    # File name in the current directory
    file_name="./client_logs/$numOfClient/output_$num.txt"

    # Use grep and awk to search and extract the float value
    if grep -q "The number of Successful response" "$file_name"; then
        num_of_response=$(grep "The number of Successful response" "$file_name" | awk '{print $6}')
        # echo "Found no of response: $num_of_response"
    fi

    if grep -q "Total time for completing the loop" "$file_name"; then
        loop_time=$(grep "Total time for completing the loop" "$file_name" | awk '{print $7}')
        # echo "Found loop time: $loop_time"
    fi
    ind_throughput=$(echo "scale=6; $num_of_response / $loop_time" | bc -l)
    overall_throughput=$(echo "scale=6; $overall_throughput + $ind_throughput" | bc -l)
done

echo "Overall throughput: $overall_throughput"

# Append the values to a csv file using >>
echo $numOfClient,$avgResponseTime,$overall_throughput >> output.csv
