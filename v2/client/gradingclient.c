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

// Initializing the constants 
const int BUFFER_SIZE = 1024; 
const int MAX_FILE_SIZE_BYTES = 4;
const int MAX_TRIES = 5;

// function to send files to client using sockfd
int send_file(int sockfd, char* file_path) {
    char buffer[BUFFER_SIZE]; //buffer to read  from  file
    bzero(buffer, BUFFER_SIZE); //initialize buffer to all NULLs
    FILE *file = fopen(file_path, "rb"); //open the file for reading, get file descriptor 
    if (!file)
    {
        perror("Error opening file");
        return -1;
    }
		
	//for finding file size in bytes
    fseek(file, 0L, SEEK_END); 
    int file_size = ftell(file);
    // printf("File size is: %d\n", file_size);
    
    //Reset file descriptor to beginning of file
    fseek(file, 0L, SEEK_SET);
		
	//buffer to send file size to server
    char file_size_bytes[MAX_FILE_SIZE_BYTES];

    //copy the bytes of the file size integer into the char buffer
    memcpy(file_size_bytes, &file_size, sizeof(file_size));
    
    //send file size to server, return -1 if error
    if (send(sockfd, &file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        perror("Error sending file size");
        fclose(file);
        return -1;
    }

	//now send the source code file 
    while (!feof(file))  //while not reached end of file
    {
    
    		//read buffer from file
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE -1, file);
        
     		//send to server
        if (send(sockfd, buffer, bytes_read+1, 0) == -1)
        {
            perror("Error sending file data");
            fclose(file);
            return -1;
        }
        
        //clean out buffer before reading into it again
        bzero(buffer, BUFFER_SIZE);
    }
    //close file
    fclose(file);
    return 0;
}



int main(int argc, char* argv[]) {

     // Checking if correct inputs are given or not
    if (argc != 6) {
        printf("Usage: ./client <serverIP:port> <sourceCodeFileTobeGraded>  <loopNum> <sleepTimeSeconds> <time-out> \n");
        return 1;
    }

    // extracting the inputs from the terminal
    char* server_ip_port = argv[1];
    char* source_code_file = argv[2];
    int loop = atoi(argv[3]);
    int sleep_time = atoi(argv[4]);
    int time_out_time = atoi(argv[5]);

    // Initializing necessary variables
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
                sleep(2);
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
            continue;
        }

        time_t now = time(0);

        // Opening the source code file
        if (send_file(client_socket, source_code_file) != 0) {
            printf("Error sending source file\n");
            close(client_socket);
            loop = loop - 1;
            error_no += 1;
            continue;
        }
        else {
            printf("Code sent for grading, waiting for response\n");
            successful_request += 1;
        }

        // setting the timer
        // This is the specific option indicating the receive timeout. It sets the maximum time a blocking recv call will wait for data to arrive.
        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            perror("setsockopt failed : ");
            close(client_socket);
            loop = loop - 1;
            error_no += 1;
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

        // Taking time after recieving response from the server
        time_t then = time(0);

        // checking for the timeout in the connection
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

        // calculating the reponse time
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
