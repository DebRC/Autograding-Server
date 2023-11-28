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



int send_file(int sockfd, char *file_path)
{
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        errorExit("ERROR :: FILE OPEN ERROR");
    }
    while (!feof(file))
    {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, file);
        if (send(sockfd, buffer, bytes_read + 1, MSG_NOSIGNAL) == -1)
        {
            fclose(file);
            errorExit("ERROR :: FILE SEND ERROR");
        }
        bzero(buffer, BUFFER_SIZE);
    }
    fclose(file);
    return 0;
}

