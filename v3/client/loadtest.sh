#!/bin/bash

# Checcking for correct arguments
if [ $# -ne 6 ];then
    echo "usage: ./loadtest.sh <IP:Port> <testFile> <numClients>  <loopNum>  <sleepTimeSeconds> <time_out>"
    exit 1
fi

# Variables used
ipPort=$1
testFile=$2
numOfClient=$3
numOfLoop=$4
sleepTime=$5
timeOut=$6

# creatinig the necessary directories
mkdir -p client_logs
mkdir -p client_logs/$numOfClient

# Initializing the variables
totalResponseTime=0.00
overall_throughput=0.00
overall_response_time=0.00
overall_error_rate=0.00
overall_request_rate=0.00
overall_timeout_rate=0.00
total_response=0

# run the utilizatoin script in the background
bash utilization_script.sh 8080 &

# compiling the client
echo "Running Makefile"
make

# Executing the client
for ((num=1; num<=$numOfClient; num++)); do
    output_file="output_$num.txt"
    ./client "$ipPort" "$testFile" "$numOfLoop" "$sleepTime" "$timeOut"> "./client_logs/$numOfClient/$output_file" &
    pids[num]=$!
done

# Wait until all the processes finishes
for ((i=1; i<=$numOfClient; i++)); do
    wait "${pids[$i]}"
done

# stop the utilization script after each client
PID=$(ps -eLf | grep utilization_script.sh | head -1 | awk '{print $2}')
echo "PID: $PID"
kill -9 $PID 

# vmstat 1 2 | tail -1 | awk '{print $13}' >> cpu_utilization_snapshots.txt
# ps -eLf | grep "./server $port_no" | head -1 | awk '{print $6}' >> thread_count_snapshots.txt


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
    total_response=$(echo "scale=2; $total_response + $num_of_response" | bc -l)
    # echo "total successful response: $total_response"
    client_total_response_time=$(echo "scale=6; $client_avg_response_time * $num_of_response" | bc -l)
    # echo "client total response time: $client_total_response_time"
    overall_response_time=$(echo "scale=2; $overall_response_time + $client_total_response_time" | bc -l)
    # echo "client overall response time: $overall_response_time"
done
# echo "number of client: $numOfClient"
avgResponseTime=0
echo "total successful response: $total_response"

# for avoiding the divide by zero exception
if [ $total_response -eq 0 ]; then
    avgResponseTime=${avgResponseTime}.00
else
    total_response=${total_response}.00
    avgResponseTime=$(echo "scale=2; $overall_response_time / $total_response" | bc -l)
fi

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
    # for avoiding the divide by zero exception
    if [ $loop_time -eq 0 ]; then
        ind_throughput=$num_of_response
    else 
        ind_throughput=$(echo "scale=2; $num_of_response / $loop_time" | bc -l)
    fi
    overall_throughput=$(echo "scale=2; $overall_throughput + $ind_throughput" | bc -l)
done

echo "Overall throughput: $overall_throughput"





# CALCULATING THE SUCCESSFUL REQUEST RATE
for ((num=1; num<=$numOfClient; num++)); do
    # File name in the current directory
    file_name="./client_logs/$numOfClient/output_$num.txt"

    # Use grep and awk to search and extract the float value
    if grep -q "The number of Successful request" "$file_name"; then
        num_of_request=$(grep "The number of Successful request" "$file_name" | awk '{print $6}')
    else 
        num_of_request=0
    fi

    if grep -q "Total time for completing the loop" "$file_name"; then
        loop_time=$(grep "Total time for completing the loop" "$file_name" | awk '{print $7}')
    else
        loop_time=0
    fi
    # for avoiding the divide by zero exception
    if [ $loop_time -eq 0 ]; then
        ind_request_rate=0
    else 
        ind_request_rate=$(echo "scale=2; $num_of_request / $loop_time" | bc -l)
        # echo "Number of response: $num_of_request"
        # echo "loop time: $loop_time"
        # echo "Individual Request rate: $ind_request_rate"
    fi
    overall_request_rate=$(echo "scale=2; $overall_request_rate + $ind_request_rate" | bc -l)
done

echo "Overall Request Rate(Goodput): $overall_request_rate"






# CALCULATING THE ERROR RATE
total_error=0
for ((num=1; num<=$numOfClient; num++)); do
    # File name in the current directory
    file_name="./client_logs/$numOfClient/output_$num.txt"

    # Use grep and awk to search and extract the float value
    if grep -q "The number of errors occurred" "$file_name"; then
        num_of_error=$(grep "The number of errors occurred" "$file_name" | awk '{print $6}')
    else 
        num_of_error=0
    fi

    if grep -q "Total time for completing the loop" "$file_name"; then
        loop_time=$(grep "Total time for completing the loop" "$file_name" | awk '{print $7}')
    else
        loop_time=0
    fi
    # for avoiding the divide by zero exception
    if [ $loop_time -eq 0 ]; then
        ind_error_rate=$num_of_error
    else 
        ind_error_rate=$(echo "scale=2; $num_of_error / $loop_time" | bc -l)
    fi
    total_error=$(echo "scale=2; $total_error + $num_of_error" | bc -l)
    overall_error_rate=$(echo "scale=2; $overall_error_rate + $ind_error_rate" | bc -l)
done

echo "Total errors: $total_error"
echo "Overall Error Rate: $overall_error_rate"






# CALCULATING THE TIMEOUT RATE
total_time_out=0
for ((num=1; num<=$numOfClient; num++)); do
    # File name in the current directory
    file_name="./client_logs/$numOfClient/output_$num.txt"

    # Use grep and awk to search and extract the float value
    if grep -q "The number of timeouts are" "$file_name"; then
        num_of_timeout=$(grep "The number of timeouts are" "$file_name" | awk '{print $6}')
    else 
        num_of_timeout=0
    fi

    if grep -q "Total time for completing the loop" "$file_name"; then
        loop_time=$(grep "Total time for completing the loop" "$file_name" | awk '{print $7}')
    else
        loop_time=0
    fi
    # for avoiding the divide by zero exception
    if [ $loop_time -eq 0 ]; then
        num_of_timeout=0
    else 
        ind_timeout_rate=$(echo "scale=2; $num_of_timeout / $loop_time" | bc -l)
        total_time_out=$(echo "scale=2; $total_time_out + $num_of_timeout" | bc -l)
    fi
    overall_timeout_rate=$(echo "scale=2; $overall_timeout_rate + $ind_timeout_rate" | bc -l)
done

echo "Total timeouts: $total_time_out"
echo "Overall Timeout Rate: $overall_timeout_rate"





# CALCULATING THE REQUEST RATE SENT
request_rate_sent=$(echo "scale=2; $overall_timeout_rate + $overall_error_rate + $overall_throughput" | bc -l)
echo "Overall Request Rate Sent: $request_rate_sent"





# AVERAGE NO OF ACTIVE THREADS
sum=0
count=0
while read -r line; do
  sum=$((sum + line))
  count=$((count + 1))
done < thread_count_snapshots.txt
average_thread=$(echo "scale=2; $sum / $count" | bc -l)
echo "Average active threads: $average_thread"
> thread_count_snapshots.txt





# AVERAGE NO OF CPU UTILIZATION
sum=0
count=0
while read -r line; do
  sum=$((sum + line))
  count=$((count + 1))
done < cpu_utilization_snapshots.txt
average_cpu=$(echo "scale=2; $sum / $count" | bc -l)
echo "Average cpu utilization(%): $average_cpu"
> cpu_utilization_snapshots.txt


# Append the values to a csv file using >>
echo $numOfClient,$avgResponseTime,$overall_throughput,$overall_request_rate,$overall_error_rate,$overall_timeout_rate,$request_rate_sent,$average_thread,$average_cpu >> output.csv
