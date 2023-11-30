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
CircularQueue clientQueue;

// Lock for file storage where grade will be stores
pthread_mutex_t fileLock;

// Conditional Lock and Mutex for the Queue
pthread_mutex_t requestQueueLock;
pthread_cond_t requestQueueCond;
pthread_mutex_t clientQueueLock;
pthread_cond_t clientQueueCond;

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

// Function to For fault tolerance
// Restores Request which has not been graded
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
            pthread_mutex_lock(&requestQueueLock);
            // printf("DEBUG 3 :: ENQUEUEING REQUEST ID\n");

            if (enqueue(&requestQueue, requestID) < 0)
            {
                pthread_mutex_unlock(&requestQueueLock);
                errorExit("Enqueue Error");
            };
            printf("Request ID = %d, is Re-Added to Queue.\n", requestID);

            // Signal to wake up a waiting thread
            pthread_cond_signal(&requestQueueCond);
            pthread_mutex_unlock(&requestQueueLock);
        }
    }
    printf("FAULT TOLERANCE DONE\n");
    return 0;
}

// Grader function.
// Dequeues the given requestID
// and grade them, store result in storage
int grader(int requestID)
{
    // Create the file names and commands for compilation process
    char *programFileName = make_program_filename(requestID);
    char *execFileName = make_exec_filename(requestID);
    char *compileOutputFileName = make_compile_output_filename(requestID);
    char *runtimeOutputFileName = make_runtime_output_filename(requestID);
    char *outputFileName = make_output_filename(requestID);
    char *outputDiffFileName = make_output_diff_filename(requestID);

    char *compileCommand = compile_command(requestID, programFileName, execFileName);
    char *runCommand = run_command(requestID, execFileName);
    char *outputCheckCommand = output_check_command(requestID, outputFileName);

    // Execute the commands
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

    // Free heap vars
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

// If client send 'new' request
int createNewRequest(int clientSockFD)
{
    // Generates a unique request id
    int requestID = generateUniqueRequestID();
    int n;

    // Make the filename using request id
    char *programFileName = make_program_filename(requestID);
    if (recv_file(clientSockFD, programFileName) != 0)
    {
        free(programFileName);
        errorExit("ERROR :: FILE RECV ERROR");
    }
    free(programFileName);

    // Respond a confirmation message to client
    n = send(clientSockFD, "I got your code file for grading\n", BUFFER_SIZE, MSG_NOSIGNAL);
    if (n < 0)
        errorExit("ERROR :: FILE SEND ERROR");

    // Lock the queue and add the request id for grading
    pthread_mutex_lock(&requestQueueLock);
    // printf("DEBUG 2 :: ENQUEUEING REQUEST ID\n");

    if (enqueue(&requestQueue, requestID) < 0)
    {
        pthread_mutex_unlock(&requestQueueLock);
        closeSocket(clientSockFD);
        errorExit("Enqueue Error");
    }
    printf("Client with FD = %d is given Request ID = %d\n", clientSockFD, requestID);

    // Signal to wake up a waiting thread
    pthread_cond_signal(&requestQueueCond);
    pthread_mutex_unlock(&requestQueueLock);

    char requestIDString[30];
    // Convert integer to string
    sprintf(requestIDString, "Your RequestID is : %d\n", requestID);

    // Respond Request ID back to client
    n = send(clientSockFD, requestIDString, BUFFER_SIZE, MSG_NOSIGNAL);
    if (n < 0)
        errorExit("ERROR :: FILE SEND ERROR");

    // Write the status to the file
    pthread_mutex_lock(&fileLock);
    n = writeStatusToFile(requestID, "0");
    pthread_mutex_unlock(&fileLock);
    if (n != 0)
    {
        errorExit("ERROR :: File Write Error");
    }
    return 0;
}

// If client send 'status' request
int checkStatusRequest(int clientSockFD, int requestID)
{
    int n;

    // Lock the file system
    pthread_mutex_lock(&fileLock);
    // Reads the status from file
    char *status = readStatusFromFile(requestID);
    pthread_mutex_unlock(&fileLock);

    // If status is not valid
    if (status == NULL)
    {
        n = send(clientSockFD, "INVALID REQUEST ID", 20, MSG_NOSIGNAL);
    }
    else
    {
        // Read remarks for that following status
        char *remarks = readRemarksFromFile(status);

        // Send the remarks
        n = send(clientSockFD, remarks, strlen(remarks), MSG_NOSIGNAL);
        if (n < 0)
        {
            errorExit("ERROR: SEND ERROR");
        }

        // Check if status is 2, 2 means compiler error
        // Status with their remarks are given in Remarks.txt
        if (strcmp(status, "2") == 0)
        {
            char *compileOutputFileName = make_compile_output_filename(requestID);
            n = send_file(clientSockFD, compileOutputFileName);
            free(compileOutputFileName);
        }
        // Check if status is 3, 3 means runtime error
        else if (strcmp(status, "3") == 0)
        {
            char *runtimeOutputFileName = make_runtime_output_filename(requestID);
            n = send_file(clientSockFD, runtimeOutputFileName);
            free(runtimeOutputFileName);
        }
        // 4 means there is a difference in output
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

// Thread function which manages clients
void *handleClients(void *arg)
{
    while (1)
    {
        int clientSockFD;
        // Lock the queue and get the next client socket to process
        pthread_mutex_lock(&clientQueueLock);
        while (isEmpty(&clientQueue))
        {
            // Wait for a signal indicating that the queue is not empty
            pthread_cond_wait(&clientQueueCond, &clientQueueLock);
        }

        // printf("DEBUG 4 :: DEQUEUEING CLIENT SOCK FD\n");

        // Deque the client socket from queue
        clientSockFD = dequeue(&clientQueue);
        if (clientSockFD < 0)
        {
            pthread_mutex_unlock(&clientQueueLock);
            errorContinue("Dequeue Error");
        }
        // printf("\n%s\n", clientSockFD);
        pthread_mutex_unlock(&clientQueueLock);

        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);

        // Get the first message i.e. status <new|status>
        int n = recv(clientSockFD, buffer, BUFFER_SIZE, 0);
        if (n < 0)
        {
            close(clientSockFD);
            continue;
        }
        char *status = strtok(buffer, ":");
        int requestID;

        // If status, then also fetch the request ID
        // from the same message
        if (strcmp(status, "status") == 0)
            requestID = atoi(strtok(NULL, ":"));

        // Call functions accordingly
        if (strcmp(status, "new") == 0)
        {
            createNewRequest(clientSockFD);
        }
        else if (strcmp(status, "status") == 0)
        {
            checkStatusRequest(clientSockFD, requestID);
        }

        // Close socket after done
        closeSocket(clientSockFD);
    }
}

// Thread function which
// handles the requests
// from the queue
void *handleRequests(void *arg)
{
    while (1)
    {
        // Lock the queue and get the next client socket to process
        pthread_mutex_lock(&requestQueueLock);
        while (isEmpty(&requestQueue))
        {
            // Wait for a signal indicating that the queue is not empty
            pthread_cond_wait(&requestQueueCond, &requestQueueLock);
        }

        // printf("DEBUG 5 :: DEQUEUEING REQUEST ID\n");

        // Get the request id
        int requestID = dequeue(&requestQueue);
        if (requestID < 0)
        {
            pthread_mutex_unlock(&requestQueueLock);
            errorContinue("Dequeue Error");
        }
        pthread_mutex_unlock(&requestQueueLock);

        printf("Request ID=%d is assigned a Thread\n", requestID);

        // Update the status of the request id in file storage
        pthread_mutex_lock(&fileLock);
        updateStatusToFile(requestID, "1");
        pthread_mutex_unlock(&fileLock);

        // Store grader response
        if (grader(requestID) == 0)
            printf("SUCCESS :: File Graded for Request ID = %d\n", requestID);
        else
            printf("ERROR :: File Cannot Be Graded for Request ID = %d\n", requestID);
    }
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

    // To disable socket binding error
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
    pthread_t queueCountThread;
    pthread_create(&queueCountThread, NULL, countQueueSize, NULL);

    // Grading thread pool
    // These threads independently grades the request
    int threadPoolSize = atoi(argv[2]);
    pthread_t requestThreads[threadPoolSize];
    pthread_t clientThreads[threadPoolSize];

    // Initialize the mutex and cond
    pthread_mutex_init(&fileLock, NULL);
    pthread_mutex_init(&requestQueueLock, NULL);
    pthread_cond_init(&requestQueueCond, NULL);
    pthread_mutex_init(&clientQueueLock, NULL);
    pthread_cond_init(&clientQueueCond, NULL);

    // Initialize Request Queue
    initQueue(&requestQueue);
    initQueue(&clientQueue);

    // Start Fault tolerance
    if (faultTolerance() < 0)
    {
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
        // Threads for request
        if (pthread_create(&requestThreads[i], NULL, handleRequests, NULL) != 0)
        {
            close(serverSockFD);
            errorExit("ERROR :: Thread creation failed");
        }

        // Threads for clients
        if (pthread_create(&clientThreads[i], NULL, handleClients, NULL) != 0)
        {
            close(serverSockFD);
            errorExit("ERROR :: Thread creation failed");
        }
    }

    // Loop which accepts connections indefinitely
    while (1)
    {
        // Accept client
        clientSockFD = accept(serverSockFD, (struct sockaddr *)&clientAddr, &clientAddrLen);

        // If accept fails
        if (clientSockFD < 0)
        {
            errorContinue("ERROR :: Client Socket Accept Failed");
        }

        printf("Accepted Client Connection From %s with FD = %d\n", inet_ntoa(clientAddr.sin_addr), clientSockFD);

        // Lock the queue and add the client socket for grading
        pthread_mutex_lock(&clientQueueLock);
        // printf("DEBUG 1 :: ENQUEUEING CLIENT SOCK FD\n");
        if (enqueue(&clientQueue, clientSockFD) < 0)
        {
            closeSocket(clientSockFD);
            pthread_mutex_unlock(&clientQueueLock);
            errorContinue("Enqueue Error");
        };

        // Signal to wake up a waiting thread
        pthread_cond_signal(&clientQueueCond);
        pthread_mutex_unlock(&clientQueueLock);
    }
    close(serverSockFD);

    return 0;
}
