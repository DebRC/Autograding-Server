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
#include "utils/helper.h"
#include "utils/circular_queue.h"
#include "utils/file_transfer.h"
#include "utils/make_filename.h"
#include "utils/system_commands.h"
#include "utils/file_update.h"

// Request Queue
CircularQueue requestQueue;
pthread_mutex_t fileLock;
pthread_mutex_t queueLock;
pthread_cond_t queueCond;

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
        int size = getSize(&requestQueue);
        fprintf(outputFile, "%d\n", size);
        fflush(outputFile); // Flush the file buffer to ensure data is written immediately
        sleep(1);           // Sleep for 10 seconds
    }
}

int faultTolerance()
{
    printf("RUNNING FAULT TOLERANCE ::\n");
    // Open the file in read mode
    FILE *file = fopen("request_status.csv", "r");
    if (file == NULL)
    {
        errorExit("Error Opening file");
    }
    // Search for the request ID in the file
    char line[256]; // Adjust the size as needed
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // printf("%s\n", line);
        int requestID = atoi(strtok(line, ","));
        char *status = strtok(NULL, ",");
        // printf("%d :: %s\n", requestID, status);
        status[strcspn(status, "\n")] = '\0';
        if ((strcmp(status, "0") == 0) || (strcmp(status, "1") == 0))
        {
            // Lock the queue and add the client socket for grading
            pthread_mutex_lock(&queueLock);
            enqueue(&requestQueue, requestID);
            printf("Request ID = %d, is Re-Added to Queue.\n", requestID);

            // Signal to wake up a waiting thread
            pthread_cond_signal(&queueCond);
            pthread_mutex_unlock(&queueLock);
        }
    }
    printf("FAULT TOLERANCE DONE\n");
    return 0;
}

int grader(int requestID)
{
    char *programFileName = make_program_filename(requestID);
    char *execFileName = make_exec_filename(requestID);
    char *compileOutputFileName = make_compile_output_filename(requestID);
    char *runtimeOutputFileName = make_runtime_output_filename(requestID);
    char *outputFileName = make_output_filename(requestID);
    char *outputDiffFileName = make_output_diff_filename(requestID);

    char *compileCommand = compile_command(requestID, programFileName, execFileName);
    char *runCommand = run_command(requestID, execFileName);
    char *outputCheckCommand = output_check_command(requestID, outputFileName);

    if (system(compileCommand) != 0)
    {
        pthread_mutex_lock(&fileLock);
        updateStatusToFile(requestID, "2");
        pthread_mutex_unlock(&fileLock);
    }
    else if (system(runCommand) != 0)
    {
        pthread_mutex_lock(&fileLock);
        updateStatusToFile(requestID, "3");
        pthread_mutex_unlock(&fileLock);
    }
    else
    {
        if (system(outputCheckCommand) != 0)
        {
            pthread_mutex_lock(&fileLock);
            updateStatusToFile(requestID, "4");
            pthread_mutex_unlock(&fileLock);
        }
        else
        {
            pthread_mutex_lock(&fileLock);
            updateStatusToFile(requestID, "5");
            pthread_mutex_unlock(&fileLock);
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

    return 0;
}

void *handleClient(void *arg)
{
    while (1)
    {
        // Lock the queue and get the next client socket to process
        pthread_mutex_lock(&queueLock);
        while (isEmpty(&requestQueue))
        {
            // Wait for a signal indicating that the queue is not empty
            pthread_cond_wait(&queueCond, &queueLock);
        }
        int requestID = dequeue(&requestQueue);
        pthread_mutex_unlock(&queueLock);

        printf("Request ID=%d is assigned a Thread\n", requestID);

        pthread_mutex_lock(&fileLock);
        updateStatusToFile(requestID, "1");
        pthread_mutex_unlock(&fileLock);

        if (grader(requestID) == 0)
            printf("SUCCESS :: File Graded for Request ID = %d\n", requestID);
        else
            printf("ERROR :: File Cannot Be Graded for Request ID = %d\n", requestID);
    }
}

// Function to generate a unique 6-digit request ID based on current time
int generateUniqueRequestID()
{
    // Get current time
    time_t currentTime = time(NULL);

    // Use only the last 6 digits of the current time
    int timePart = (int)(currentTime % 1000000);

    // Generate a random 3-digit number
    int randomPart = rand() % 1000;

    // Combine the time and random parts to form a 9-digit request ID
    int requestID = timePart * 1000 + randomPart;

    return requestID;
}

int createNewRequest(int clientSockFD)
{
    int requestID = generateUniqueRequestID();
    int n;
    char *programFileName = make_program_filename(requestID);
    if (recv_file(clientSockFD, programFileName) != 0)
    {
        free(programFileName);
        errorExit("ERROR :: FILE RECV ERROR");
    }
    free(programFileName);

    n = send(clientSockFD, "I got your code file for grading\n", 33, MSG_NOSIGNAL);
    if (n < 0)
        errorExit("ERROR :: FILE SEND ERROR");

    // Lock the queue and add the client socket for grading
    pthread_mutex_lock(&queueLock);
    enqueue(&requestQueue, requestID);
    printf("Client with FD = %d is given Request ID = %d\n", clientSockFD, requestID);

    // Signal to wake up a waiting thread
    pthread_cond_signal(&queueCond);
    pthread_mutex_unlock(&queueLock);

    char requestIDString[30];
    // Convert integer to string
    sprintf(requestIDString, "Your RequestID is : %d\n", requestID);

    n = send(clientSockFD, requestIDString, strlen(requestIDString), MSG_NOSIGNAL);
    if (n < 0)
        errorExit("ERROR :: FILE SEND ERROR");

    pthread_mutex_lock(&fileLock);
    n = writeStatusToFile(requestID, "0");
    pthread_mutex_unlock(&fileLock);
    if (n != 0)
    {
        errorExit("ERROR :: File Write Error");
    }
    return 0;
}

int checkStatusRequest(int clientSockFD)
{
    int n;
    int requestID;
    n = recv(clientSockFD, &requestID, sizeof(requestID), 0);
    if (n < 0)
    {
        errorExit("ERROR: RECV ERROR");
    }
    pthread_mutex_lock(&fileLock);
    char *status = readStatusFromFile(requestID);
    pthread_mutex_unlock(&fileLock);
    if (status == NULL)
    {
        n = send(clientSockFD, "INVALID REQUEST ID", 20, MSG_NOSIGNAL);
    }
    else
    {
        char *remarks = readRemarksFromFile(status);
        n = send(clientSockFD, remarks, strlen(remarks), MSG_NOSIGNAL);
        if (n < 0)
        {
            errorExit("ERROR: SEND ERROR");
        }
        if (strcmp(status, "2") == 0)
        {
            char *compileOutputFileName = make_compile_output_filename(requestID);
            n = send_file(clientSockFD, compileOutputFileName);
            free(compileOutputFileName);
        }
        else if (strcmp(status, "3") == 0)
        {
            char *runtimeOutputFileName = make_runtime_output_filename(requestID);
            n = send_file(clientSockFD, runtimeOutputFileName);
            free(runtimeOutputFileName);
        }
        else if (strcmp(status, "4") == 0)
        {
            char *outputDiffFileName = make_output_diff_filename(requestID);
            n = send_file(clientSockFD, outputDiffFileName);
            free(outputDiffFileName);
        }
    }
    if (n < 0)
    {
        errorExit("ERROR: SEND ERROR");
    }

    printf("Status Returned for Client with FD = %d with Request ID = %d\n", clientSockFD, requestID);

    return 0;
}

int getRequest(int clientSockFD)
{
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    // new or status_check
    int n = recv(clientSockFD, buffer, BUFFER_SIZE, 0);
    if (n < 0)
        return -1;
    if (strcmp(buffer, "new") == 0)
        return createNewRequest(clientSockFD);
    else if (strcmp(buffer, "status") == 0)
        return checkStatusRequest(clientSockFD);
    return -1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
        errorExit("Usage: <portNumber> <threadPoolSize>");

    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Server and Client socket necessary variables
    int serverSockFD, serverPortNo;
    struct sockaddr_in serverAddr, clientAddr;
    int clientSockFD;

    // Make the server socket
    serverSockFD = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSockFD < 0)
        errorExit("ERROR :: Socket Opening Failed");

    if (setsockopt(serverSockFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        errorExit("ERROR :: setsockopt (SO_REUSEADDR) Failed");

    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverPortNo = atoi(argv[1]);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(serverPortNo);

    // Get address size
    int clientAddrLen = sizeof(clientAddr);

    // Thread to count queue size
    // pthread_t queueCountThread;
    // pthread_create(&queueCountThread, NULL, countQueueSize, NULL);

    int threadPoolSize = atoi(argv[2]);
    pthread_t threads[threadPoolSize];

    // Initialize the mutex and cond
    pthread_mutex_init(&fileLock, NULL);
    pthread_mutex_init(&queueLock, NULL);
    pthread_cond_init(&queueCond, NULL);

    // Initialize Request Queue
    initQueue(&requestQueue);

    if(faultTolerance()<0){
        errorExit("ERROR :: While running fault tolerance");
    }

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

        getRequest(clientSockFD);

        closeSocket(clientSockFD);
    }
    close(serverSockFD);

    return 0;
}
