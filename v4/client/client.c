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
#include "utils/helper.h"
#include "utils/file_transfer.h"

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
        if (send(client_socket, argv[1], BUFFER_SIZE, 0) < 0) {
            perror("Error sending new flag : \n");
            close(client_socket);
        }

        
        // sleep(5);

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
            // printf("rcv_bytes: %d\n",rcv_bytes);
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

        char buffer[BUFFER_SIZE];
        memset(buffer,0,BUFFER_SIZE);

        sprintf(buffer, "%s:%d", status, requestID);

        if(send(client_socket, buffer, BUFFER_SIZE, 0) < 0) {
            perror("Error sending status flag : \n");
            close(client_socket);
        }

        //buffer for reading server response
        int rcv_bytes;
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
            printf("Server Response: ");
            printf("%s\n", buffer);
            memset(buffer,0,BUFFER_SIZE);
        }
    }
    else {
        printf("Wrong status");
    }
    return 0;
}
