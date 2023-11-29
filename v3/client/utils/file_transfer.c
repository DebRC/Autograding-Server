#include "file_transfer.h"
#include "helper.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>

int recv_file(int sockfd, char *file_path)
{
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    FILE *file = fopen(file_path, "wb");
    if (!file)
    {
        errorExit("ERROR :: FILE OPEN ERROR");
    }
    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    if (recv(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        fclose(file);
        errorExit("ERROR :: FILE RECV ERROR");
    }

    int file_size;
    memcpy(&file_size, file_size_bytes, sizeof(file_size_bytes));

    printf("File size is: %d\n", file_size);

    size_t bytes_read = 0, total_bytes_read = 0;
    while (true)
    {
        bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0);
        total_bytes_read += bytes_read;

        if (bytes_read <= 0)
        {
            fclose(file);
            errorExit("ERROR :: FILE RECV ERROR");
        }
        fwrite(buffer, 1, bytes_read, file);
        bzero(buffer, BUFFER_SIZE);
        if (total_bytes_read >= file_size)
            break;
    }
    fclose(file);
    return 0;
}

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
    if (send(sockfd, &file_size_bytes, sizeof(file_size_bytes), MSG_NOSIGNAL) == -1)
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
        if (send(sockfd, buffer, bytes_read+1, MSG_NOSIGNAL) == -1)
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