#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdbool.h>
#include "helper/helper.h"


const int MAX_TRIES = 5;
const int time_out_time = 5;

int main(int argc, char* argv[]) {

    // Usage
    if (argc != 4) {
        printf("Usage: ./client <new|status> <serverIP:port> <sourceCodeFileTobeGraded|requestID>\n");

    }
    
    char* server_ip_port = argv[2];
    int client_socket;

    // Set Timeout timer
    struct timeval timeout;
    timeout.tv_sec = time_out_time;
    timeout.tv_usec = 0;

    // Parse server IP and port
    char* server_ip = strtok(server_ip_port, ":");
    int server_port = atoi(strtok(NULL, ":"));

    // Set up server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);

    if (strcmp(argv[1],"new") == 0) {
        
        // taking source code file
        char* source_code_file = argv[3];
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket < 0) {
            perror("Error creating socket");
            close(client_socket);
        }

        // Connect to the server
        int tries = 0;
        while(true) {
            if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
                break;
            }
            else {
                sleep(1);
                tries += 1;
                if (tries == MAX_TRIES) {
                    perror("Server not responding\n");
                    close(client_socket);
                    break;
                }
            }
        }

        // sending new flag to the server
        if (send(client_socket, argv[1], sizeof(argv[1]), 0) < 0) {
            perror("Error sending new flag : \n");
            close(client_socket);
        }

        // Opening the source code file
        if (send_file(client_socket, source_code_file) != 0) {
            perror("Error sending source file\n");
            close(client_socket);
        }
        else {
            printf("Code sent for grading, waiting for response\n");
        }
        int rcv_bytes;

        //buffer for reading server response
        char buffer[BUFFER_SIZE];
        memset(buffer,0,BUFFER_SIZE);
        while (true)
        {
            //read server response
            rcv_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
            printf("rcv_bytes: %d\n",rcv_bytes);
            if (rcv_bytes <= 0) {
                // printf("in if rcv_bytes: %d\n",rcv_bytes);
                break;
            }
            // 
            printf("Server Response: ");
            printf("%s\n", buffer);
            memset(buffer,0,BUFFER_SIZE);
        }
        close(client_socket);
    }
    else if (strcmp(argv[1], "status") == 0) {

        // taking the status of client
        int requestID = atoi(argv[3]);
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket < 0) {
            perror("Error creating socket");
            close(client_socket);
        }

        // Connect to the server
        int tries = 0;
        while(true) {
            if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
                break;
            }
            else {
                sleep(1);
                tries += 1;
                if (tries == MAX_TRIES) {
                    perror("Server not responding\n");
                    close(client_socket);
                    break;
                }
            }
        }

        char* status = argv[1];
        if(send(client_socket, status, sizeof(status), 0) < 0) {
            perror("Error sending status flag : \n");
            close(client_socket);
        }

        // sending new flag to the server
        if (send(client_socket, &requestID, sizeof(requestID), 0) < 0) {
            perror("Error sending requestID : \n");
            close(client_socket);
        }

        //buffer for reading server response
        int rcv_bytes;
        char buffer[BUFFER_SIZE];
        memset(buffer,0,BUFFER_SIZE);
        while (true)
        {
            //read server response
            rcv_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
            // printf("rcv_bytes: %d\n",rcv_bytes);
            if (rcv_bytes <= 0) {
                // printf("in if rcv_bytes: %d\n",rcv_bytes);
                break;
            }
            // printf("rcv_bytes: %d\n",rcv_bytes);
            printf("Server Response: ");
            printf("%s\n", buffer);
            memset(buffer,0,BUFFER_SIZE);
            // sleep(1);
        }
    }
    else {
        printf("Wrong status");
    }

    // if (strcmp(argv[1], "status") == 0) {

    // }

    //     // Create socket
        

        

    //     // If error happens while connecting then continue to loop
    //     if(server_error == 1) {
    //         sleep(1);
    //         continue;
    //     }

    //     time_t now = time(0);

        

    //     // setting the timer
    //     if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    //         perror("setsockopt failed : ");
    //         close(client_socket);
    //         loop = loop - 1;
    //         error_no += 1;
    //         sleep(1);
    //         continue;
    //     }

        

    //     // printf("rcv_bytes: %d\n",rcv_bytes);

    //     time_t then = time(0);

    //     if (rcv_bytes < 0) {
    //         if (errno == EWOULDBLOCK || errno == EAGAIN) {
    //             num_of_timeout += 1;
    //             perror("Time out : No response from server : ");
    //         }
    //         else {
    //             error_no += 1;
    //             perror("No response from server : ");
    //         }    
    //     }
    //     else {
    //         printf("response successfully recieved\n");
    //         successful_response += 1;
    //     }
    //     time_t diff = then - now;
    //     printf("Response Time: %lld\n\n", (long long) diff);

    //     // total time taken
    //     total_time += (long long) diff;;
    //     loop = loop - 1;

    //     //closing the client socket
    //     close(client_socket);
    //     sleep(sleep_time);
    // }
    // time_t loop_end = time(0);

    // // printing all necessary outputs
    // printf("The number of Successful response: %d\n", successful_response);
    // if (successful_response == 0) 
    //     printf("Average response time: %f Seconds\n", (float)total_time);
    // else
    //     printf("Average response time: %f Seconds\n", total_time / (float)successful_response);
    // printf("Total response time: %lld Seconds\n", total_time);
    // printf("Total time for completing the loop: %lld Seconds\n", (long long) (loop_end - loop_start));
    // printf("The number of Successful request: %d\n", successful_request);
    // printf("The number of errors occurred: %d\n", error_no);
    // printf("The number of timeouts are: %d\n", num_of_timeout);
    return 0;
}
