#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdbool.h>
#include "utils/helper.h"
#include "utils/circular_queue.h"
#include "utils/file_transfer.h"
#include "utils/make_filename.h"
#include "utils/system_commands.h"

// initializing the constats
const int MAX_TRIES = 5;

int main(int argc, char* argv[]) {

    // Checking if the inputs are correct
    if (argc != 6) {
        printf("Usage: ./client <serverIP:port> <sourceCodeFileTobeGraded>  <loopNum> <sleepTimeSeconds> <time-out> \n");
        return 1;
    }

    // Taking the input arguments
    char* server_ip_port = argv[1];
    char* source_code_file = argv[2];
    int loop = atoi(argv[3]);
    int sleep_time = atoi(argv[4]);
    int time_out_time = atoi(argv[5]);

    // Initializing the variables used
    int client_socket;
    struct sockaddr_in server_addr;
    long long total_time = 0;
    int successful_response = 0;
    int error_flag;
    int successful_request = 0;
    int num_of_timeout = 0;
    int error_no = 0;

    // Set Timeout timer
    struct timeval timeout;
    timeout.tv_sec = time_out_time;
    timeout.tv_usec = 0;

    // Parse server IP and port
    char* server_ip = strtok(server_ip_port, ":");
    int server_port = atoi(strtok(NULL, ":"));

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);
    // inet_pton(AF_INET.)

    // Starting loop.
    time_t loop_start = time(0);
    while(loop > 0) {

        // Create socket
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket < 0) {
            perror("Error creating socket");
            close(client_socket);
            loop = loop - 1;
            error_no += 1;
            sleep(1);
            continue;
        }

        // Connect to the server
        int tries = 0;
        int server_error = 0;
        while(true) {
            if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
                break;
            }
            else {
                sleep(1);
                tries += 1;
                if (tries == MAX_TRIES) {
                    printf("Server not responding\n");
                    close(client_socket);
                    loop = loop - 1;
                    server_error = 1;
                    error_no += 1;
                    break;
                }
            }
        }

        // If error happens while connecting then continue to loop
        if(server_error == 1) {
            sleep(1);
            continue;
        }

        time_t now = time(0);

        // Opening the source code file
        if (send_file(client_socket, source_code_file) != 0) {
            printf("Error sending source file\n");
            close(client_socket);
            loop = loop - 1;
            error_no += 1;
            sleep(1);
            continue;
        }
        else {
            printf("Code sent for grading, waiting for response\n");
            successful_request += 1;
        }

        // setting the timer
        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            perror("setsockopt failed : ");
            close(client_socket);
            loop = loop - 1;
            error_no += 1;
            sleep(1);
            continue;
        }

        int rcv_bytes;
        //buffer for reading server response
        char buffer[BUFFER_SIZE];
        memset(buffer,0,BUFFER_SIZE);
        while (true)
        {
            //read server response
            rcv_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (rcv_bytes <= 0) {
                // printf("in if rcv_bytes: %d\n",rcv_bytes);
                break;
            }
            // printf("rcv_bytes: %d\n",rcv_bytes);
            printf("Server Response: ");
            printf("%s\n", buffer);
            memset(buffer,0,BUFFER_SIZE);
        }

        // printf("rcv_bytes: %d\n",rcv_bytes);

        time_t then = time(0);

        // Checking for the time-out errors
        if (rcv_bytes < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                num_of_timeout += 1;
                perror("Time out : No response from server : ");
            }
            else {
                error_no += 1;
                perror("No response from server : ");
            }    
        }
        else {
            printf("response successfully recieved\n");
            successful_response += 1;
        }
        time_t diff = then - now;
        printf("Response Time: %lld\n\n", (long long) diff);

        // total time taken
        total_time += (long long) diff;;
        loop = loop - 1;

        //closing the client socket
        close(client_socket);
        sleep(sleep_time);
    }
    time_t loop_end = time(0);

    // printing all necessary outputs
    printf("The number of Successful response: %d\n", successful_response);
    if (successful_response == 0) 
        printf("Average response time: %f Seconds\n", (float)total_time);
    else
        printf("Average response time: %f Seconds\n", total_time / (float)successful_response);
    printf("Total response time: %lld Seconds\n", total_time);
    printf("Total time for completing the loop: %lld Seconds\n", (long long) (loop_end - loop_start));
    printf("The number of Successful request: %d\n", successful_request);
    printf("The number of errors occurred: %d\n", error_no);
    printf("The number of timeouts are: %d\n", num_of_timeout);
    return 0;
}
