#!/bin/bash

# Checcking for correct arguments
if [ $# -ne 2 ];then
    echo "usage: ./loadtest.sh <IP:Port> <testFile>"
    exit 1
fi
#!/bin/bash

server_address=$1
file_to_grade=$2

echo "Running Makefile"
make

# Step 1: Send the file for grading and get the request ID
response=$(./client new $server_address $file_to_grade)
request_id=$(echo "$response" | grep -o 'Your RequestID is : [0-9]*' | awk '{print $NF}')

echo "File sent for grading. Request ID: $request_id"

start_time=$(date +%s)

tries=0
max_tries=50

# Step 2: Continuously ask for the status
while true; do
    status_response=$(./client status $server_address $request_id)

    echo "Status for Request ID $request_id: $status_response"

    # Check if the status response indicates completion
    if [[ $status_response == *"Grading Complete"* ]]; then
        echo "Grading completed for Request ID: $request_id"
        break
    fi
    
    # Add a delay before the next status request
    sleep 2
    tries=$((tries+1))
    if [ $tries -eq $max_tries ]; then
        exit 1
    fi
done

end_time=$(date +%s)


# Calculate the time taken
time_taken=$((end_time - start_time))
echo "Time taken for grading: $time_taken seconds"
