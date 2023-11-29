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

// Request Queue
CircularQueue requestQueue;
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

int grader(int clientSockFD)
{
    int threadID = pthread_self();

    int n;
    char *programFileName = make_program_filename(threadID);    // creating the file name with unique id
    if (recv_file(clientSockFD, programFileName) != 0)  // recieving the file from the client
    {
        free(programFileName);
        errorExit("ERROR :: FILE RECV ERROR");
    }
    n = send(clientSockFD, "I got your code file for grading\n", 33, MSG_NOSIGNAL); //send the response back to the client
    if (n < 0)
    {
        free(programFileName);
        errorExit("ERROR :: FILE SEND ERROR");
    }

    // creating all the necessary commands and file names
    char *execFileName = make_exec_filename(threadID);
    char *compileOutputFileName = make_compile_output_filename(threadID);
    char *runtimeOutputFileName = make_runtime_output_filename(threadID);
    char *outputFileName = make_output_filename(threadID);
    char *outputDiffFileName = make_output_diff_filename(threadID);

    char *compileCommand = compile_command(threadID, programFileName, execFileName);
    char *runCommand = run_command(threadID, execFileName);
    char *outputCheckCommand = output_check_command(threadID, outputFileName);

    // check if compile command runs successfully
    if (system(compileCommand) != 0)
    {
        n = send(clientSockFD, "COMPILER ERROR", BUFFER_SIZE, MSG_NOSIGNAL); // send back response to the client
        // sleep(1);
        if (n >= 0)
        {
            n = send_file(clientSockFD, compileOutputFileName); // send the compiler error to the client
        }
    }

    // chceck if execute command is running properly
    else if (system(runCommand) != 0)
    {
        n = send(clientSockFD, "RUNTIME ERROR", BUFFER_SIZE, MSG_NOSIGNAL);  // send back the response to the client
        if (n >= 0)
            n = send_file(clientSockFD, runtimeOutputFileName); // send the runtime error to the client
    }
    else
    {
        //check the output matches with the expected output or not
        if (system(outputCheckCommand) != 0)
        {
            n = send(clientSockFD, "OUTPUT ERROR", BUFFER_SIZE, MSG_NOSIGNAL);   // send respond to the client MSG_NOSIGNAL flag fixes the broken pipe error
            if (n >= 0)
                n = send_file(clientSockFD, outputDiffFileName);    // send the difference to the client
        }
        else
        {
            n = send(clientSockFD, "PROGRAM RAN", BUFFER_SIZE, MSG_NOSIGNAL);    // send the client about successful executioon
        }
    }

    // free all the variables used
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
    // check if arguments are correct or not
    if (argc != 3)
        errorExit("Usage: <portNumber> <threadPoolSize>");

    // Server and Client socket necessary variables
    int serverSockFD, serverPortNo;
    struct sockaddr_in serverAddr, clientAddr;
    int clientSockFD;
    
    // Make the server socket
    serverSockFD = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSockFD < 0)
        errorExit("ERROR :: Socket Opening Failed");

    // implementing the timer error
    if (setsockopt(serverSockFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        error("ERROR :: setsockopt (SO_REUSEADDR) Failed");

    // initializing the serverAddr structure
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
    initQueue(&requestQueue);

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
        
        enqueue(&requestQueue, clientSockFD);

        // Signal to wake up a waiting thread
        pthread_cond_signal(&queueCond);
        pthread_mutex_unlock(&queueLock);
    }

    printf("CLOSING SERVER");
    close(serverSockFD);

    return 0;
}
