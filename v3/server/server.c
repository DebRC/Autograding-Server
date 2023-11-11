#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "circular_queue.h"

#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE_BYTES 4
#define MAX_CLIENTS 10

#define error(msg)   \
    {                \
        perror(msg); \
    }

#define errorExit(msg) \
    {                  \
        perror(msg);   \
        return -1;     \
    }

#define errorContinue(msg) \
    {                      \
        perror(msg);       \
        continue;          \
    }

#define closeSocket(clientSockFD)                                    \
    {                                                                \
        close(clientSockFD);                                         \
        printf("Closed Client Socket with FD = %d\n", clientSockFD); \
    }

// Request Queue
CircularQueue requestQueue;
pthread_mutex_t queueLock;
pthread_cond_t queueCond;

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

char *compile_command(int id, char *programFile, char *execFile)
{

    char *s;
    char s1[20];

    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    strcpy(s, "g++ -o ");
    strcat(s, execFile);
    strcat(s, "  ");
    strcat(s, programFile);
    strcat(s, " 2> logs/outputs/compiler_err");
    sprintf(s1, "%d", id);
    strcat(s, s1);
    // printf("%s\n", s);
    return s;
}

char *run_command(int id, char *execFile)
{
    char *s;
    char s1[20];

    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    sprintf(s1, "%d", id);

    strcpy(s, "./");
    strcat(s, execFile);
    strcat(s, " > logs/outputs/out");
    strcat(s, s1);
    strcat(s, " 2> logs/outputs/runtime_err");
    strcat(s, s1);
    // printf("%s\n", s);
    return s;
}

char *output_check_command(int id, char *outputFile)
{
    char *s;
    char s1[20];

    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    sprintf(s1, "%d", id);

    strcpy(s, "diff -Z ");
    strcat(s, outputFile);
    strcat(s, " expected_output.txt");
    strcat(s, " > logs/outputs/output_diff");
    strcat(s, s1);
    // printf("\n%s\n",s);
    return s;
}

char *make_program_filename(int id)
{

    char *s;
    char s1[20];

    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    sprintf(s1, "%d", id);
    strcpy(s, "logs/files/file");
    strcat(s, s1);
    strcat(s, ".cpp");
    return s;
}

char *make_exec_filename(int id)
{
    char *s;
    char s1[20];

    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    sprintf(s1, "%d", id);
    strcpy(s, "logs/files/prog");
    strcat(s, s1);
    return s;
}

char *make_compile_output_filename(int id)
{
    char *s;
    char s1[20];
    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    sprintf(s1, "%d", id);
    strcpy(s, "logs/outputs/compiler_err");
    strcat(s, s1);
    return s;
}

char *make_runtime_output_filename(int id)
{
    char *s;
    char s1[20];
    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    sprintf(s1, "%d", id);
    strcpy(s, "logs/outputs/runtime_err");
    strcat(s, s1);
    return s;
}

char *make_output_filename(int id)
{
    char *s;
    char s1[20];
    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    sprintf(s1, "%d", id);
    strcpy(s, "logs/outputs/out");
    strcat(s, s1);
    return s;
}

char *make_output_diff_filename(int id)
{
    char *s;
    char s1[20];
    s = malloc(200 * sizeof(char));
    memset(s, 0, sizeof(s));
    memset(s1, 0, sizeof(s1));
    sprintf(s1, "%d", id);
    strcpy(s, "logs/outputs/output_diff");
    strcat(s, s1);
    return s;
}

// Function to count and write queue size to a file
void *countQueueSize(void *arg)
{
    FILE *outputFile; // File to write queue size
    outputFile = fopen("logs/queue_size.log", "w");
    if (outputFile == NULL)
    {
        error("Failed to open the output file");
        return (void *)NULL;
    }
    while (1)
    {
        int size = countItems(&requestQueue);
        fprintf(outputFile, "%d\n", size);
        fflush(outputFile); // Flush the file buffer to ensure data is written immediately
        sleep(1);           // Sleep for 10 seconds
    }
}

int grader(int clientSockFD)
{
    int threadID = pthread_self();

    int n;
    char *programFileName = make_program_filename(threadID);
    if (recv_file(clientSockFD, programFileName) != 0)
    {
        free(programFileName);
        errorExit("ERROR :: FILE RECV ERROR");
    }
    n = send(clientSockFD, "I got your code file for grading\n", 33, MSG_NOSIGNAL);
    if (n < 0)
    {
        free(programFileName);
        errorExit("ERROR :: FILE SEND ERROR");
    }

    char *execFileName = make_exec_filename(threadID);
    char *compileOutputFileName = make_compile_output_filename(threadID);
    char *runtimeOutputFileName = make_runtime_output_filename(threadID);
    char *outputFileName = make_output_filename(threadID);
    char *outputDiffFileName = make_output_diff_filename(threadID);

    char *compileCommand = compile_command(threadID, programFileName, execFileName);
    char *runCommand = run_command(threadID, execFileName);
    char *outputCheckCommand = output_check_command(threadID, outputFileName);

    if (system(compileCommand) != 0)
    {
        n = send(clientSockFD, "COMPILER ERROR", 15, MSG_NOSIGNAL);
        sleep(1);
        if (n >= 0)
        {
            n = send_file(clientSockFD, compileOutputFileName);
        }
    }
    else if (system(runCommand) != 0)
    {
        n = send(clientSockFD, "RUNTIME ERROR", 14, MSG_NOSIGNAL);
        if (n >= 0)
            n = send_file(clientSockFD, runtimeOutputFileName);
    }
    else
    {
        if (system(outputCheckCommand) != 0)
        {
            n = send(clientSockFD, "OUTPUT ERROR", 14, MSG_NOSIGNAL);
            if (n >= 0)
                n = send_file(clientSockFD, outputDiffFileName);
        }
        else
        {
            n = send(clientSockFD, "PROGRAM RAN", 12, MSG_NOSIGNAL);
        }
    }

    free(programFileName);
    free(execFileName);
    free(compileOutputFileName);
    free(runtimeOutputFileName);
    free(outputFileName);
    free(outputDiffFileName);
    free(compileCommand);
    free(runCommand);
    free(outputCheckCommand);

    if (n < 0)
        errorExit("ERROR :: FILE SEND ERROR");
    return 0;
}

void *handleClient(void *arg)
{
    while (1)
    {
        int clientSockFD;
        // Lock the queue and get the next client socket to process
        pthread_mutex_lock(&queueLock);
        while (isEmpty(&requestQueue))
        {
            // Wait for a signal indicating that the queue is not empty
            pthread_cond_wait(&queueCond, &queueLock);
        }
        clientSockFD = dequeue(&requestQueue);
        pthread_mutex_unlock(&queueLock);

        printf("Client Socket with FD=%d is assigned a Thread\n", clientSockFD);

        if (grader(clientSockFD) == 0)
            printf("SUCCESS :: Client File Graded for Client Socket with FD = %d\n", clientSockFD);
        else
            printf("ERROR :: Client File Cannot Be Graded for Client Socket with FD = %d\n", clientSockFD);
        closeSocket(clientSockFD);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
        errorExit("Usage: <portNumber> <threadPoolSize> <requestQueueSize>");

    // Server and Client socket necessary variables
    int serverSockFD, serverPortNo;
    struct sockaddr_in serverAddr, clientAddr;
    int clientSockFD;
    
    // Make the server socket
    serverSockFD = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSockFD < 0)
        errorExit("ERROR :: Socket Opening Failed");

    if (setsockopt(serverSockFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        error("ERROR :: setsockopt (SO_REUSEADDR) Failed");

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverPortNo = atoi(argv[1]);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(serverPortNo);

    // Get address size
    int clientAddrLen = sizeof(clientAddr);

    // Thread to count queue size
    pthread_t queueCountThread;
    pthread_create(&queueCountThread, NULL, countQueueSize, NULL);

    int threadPoolSize = atoi(argv[2]);
    pthread_t threads[threadPoolSize];

    // Initialize the mutex and cond
    pthread_mutex_init(&queueLock, NULL);
    pthread_cond_init(&queueCond, NULL);

    // Initialize Request Queue
    int requestQueueSize = atoi(argv[3]);
    initQueue(&requestQueue, requestQueueSize);

    // Binding the server socket
    if (bind(serverSockFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        errorExit("ERROR :: Socket Binding Failed");
    }
    printf("Server is Live on Port :: %d\n", serverPortNo);

    // Listening to the server socket
    if (listen(serverSockFD, MAX_CLIENTS) < 0)
    {
        errorExit("ERROR :: Socket Listening Failed");
    }

    // Create thread pool
    for (int i = 0; i < threadPoolSize; i++)
    {
        if (pthread_create(&threads[i], NULL, handleClient, NULL) != 0)
        {
            close(serverSockFD);
            errorExit("ERROR :: Thread creation failed");
        }
    }

    while (1)
    {
        clientSockFD = accept(serverSockFD, (struct sockaddr *)&clientAddr, &clientAddrLen);

        // If accept fails
        if (clientSockFD < 0)
        {
            errorContinue("ERROR :: Client Socket Accept Failed");
        }

        printf("Accepted Client Connection From %s with FD = %d\n", inet_ntoa(clientAddr.sin_addr), clientSockFD);

        // Lock the queue and add the client socket for grading
        pthread_mutex_lock(&queueLock);
        if (isFull(&requestQueue))
        {
            pthread_mutex_unlock(&queueLock);
            closeSocket(clientSockFD);
            errorContinue("ERROR :: Request Queue Full");
        }
        enqueue(&requestQueue, clientSockFD);

        // Signal to wake up a waiting thread
        pthread_cond_signal(&queueCond);
        pthread_mutex_unlock(&queueLock);
    }

    printf("CLOSING SERVER");
    close(serverSockFD);

    return 0;
}
