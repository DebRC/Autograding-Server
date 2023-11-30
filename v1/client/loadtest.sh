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

# creating the needy directories
mkdir -p client_logs
mkdir -p client_logs/$numOfClient

# Initialize the variables
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
        # echo "Client Average response time: $client_avg_response_time"
    fi
    if grep -q "The number of Successful response" "$file_name"; then
        num_of_response=$(grep "The number of Successful response" "$file_name" | awk '{print $6}')
        # echo "Number of Successful response: $num_of_response"
    fi

    # calculating the overall response time
    total_response=$(echo "scale=6; $total_response + $num_of_response" | bc -l)
    client_total_response_time=$(echo "scale=6; $client_avg_response_time * $num_of_response" | bc -l)
    overall_response_time=$(echo "scale=6; $overall_response_time + $client_total_response_time" | bc -l)
done

# If total response time somehow comes zero make average response time also to zero
# calculating the average response time
if [ $total_response -eq 0 ]; then
    $avgResponseTime=0.000000
else
    total_response=${total_response}.000000
    avgResponseTime=$(echo "scale=6; $overall_response_time / $total_response" | bc -l)
fi
echo "Average response time: $avgResponseTime"
echo "Total Successful response: $total_response"



# CALCULATING THE OVERALL THROUGHPUT
for ((num=1; num<=$numOfClient; num++)); do
    # File name in the current directory
    file_name="./client_logs/$numOfClient/output_$num.txt"

    # Use grep and awk to search and extract the float value
    if grep -q "The number of Successful response" "$file_name"; then
        num_of_response=$(grep "The number of Successful response" "$file_name" | awk '{print $6}')
        # echo "Number of Successful response: $num_of_response"
    fi

    if grep -q "Total time for completing the loop" "$file_name"; then
        loop_time=$(grep "Total time for completing the loop" "$file_name" | awk '{print $7}')
        # echo "Loop time: $loop_time"
    fi

    # To avoid divide by zero exception
    if [ $loop_time -eq 0 ]; then
        $ind_throughput=0.000000
    else
        ind_throughput=$(echo "scale=6; $num_of_response / $loop_time" | bc -l) # calculating the individual throughput
    fi
    overall_throughput=$(echo "scale=6; $overall_throughput + $ind_throughput" | bc -l) # calculating the overall throughput
done


echo "Overall throughput: $overall_throughput"

# Append the values to a csv file using >>
echo $numOfClient,$avgResponseTime,$overall_throughput >> output.csv
