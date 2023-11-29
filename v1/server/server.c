#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>

// defining the constants used
#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE_BYTES 4
#define MAX_CLIENTS 3

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

int reqID = 0;

// Function to recieve file from the client
int recv_file(int sockfd, char *file_path)
{
    char buffer[BUFFER_SIZE];   // Initializing buffer to read
    bzero(buffer, BUFFER_SIZE); // Initialize buffer to all NULLs
    FILE *file = fopen(file_path, "wb");    // Opening the file from the file path given
    if (!file)
    {
        errorExit("ERROR :: FILE OPEN ERROR");
    }

    // reveiving the file size from the client
    char file_size_bytes[MAX_FILE_SIZE_BYTES];
    if (recv(sockfd, file_size_bytes, sizeof(file_size_bytes), 0) == -1)
    {
        fclose(file);
        errorExit("ERROR :: FILE RECV ERROR");
    }

    int file_size;

    // copying the file size from the char array
    memcpy(&file_size, file_size_bytes, sizeof(file_size_bytes));

    printf("File size is: %d\n", file_size);

    size_t bytes_read = 0, total_bytes_read = 0;

    // recieving the actual file from the client and writes to the opened file
    while (true)
    {
        bytes_read = recv(sockfd, buffer, BUFFER_SIZE, 0);
        total_bytes_read += bytes_read;

        if (bytes_read <= 0)
        {
            fclose(file);
            errorExit("ERROR :: FILE RECV ERROR");
        }

        // write the source code to the file
        fwrite(buffer, 1, bytes_read, file);
        bzero(buffer, BUFFER_SIZE);
        if (total_bytes_read >= file_size)
            break;
    }
    fclose(file);
    return 0;
}

// Function to send to client
int send_file(int sockfd, char *file_path)
{
    char buffer[BUFFER_SIZE];   // Initializing the buffer
    bzero(buffer, BUFFER_SIZE);
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        errorExit("ERROR :: FILE OPEN ERROR");
    }

    // Send necessary infromation to the client
    while (!feof(file))
    {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE - 1, file);
        if (send(sockfd, buffer, bytes_read + 1, 0) == -1)
        {
            fclose(file);
            errorExit("ERROR :: FILE SEND ERROR");
        }
        bzero(buffer, BUFFER_SIZE);
    }
    fclose(file);
    return 0;
}


// Function to create the compile command
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

// Function to create the run command
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

// Function to create output check command
char *output_check_command(int id, char *outputFile){
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

// Function to create program filename
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

// Function to create the execute filename
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

// Function to create the compile error file 
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

// Function to make the runtime error file
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

// Function to create the output file 
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

// Function to create the output diff file
char *make_output_diff_filename(int id){
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

// The main grader function
int grader(int clientSockFD)
{
    int n;
    char *programFileName = make_program_filename(reqID);   // creating the program file name with unique id
    if (recv_file(clientSockFD, programFileName) != 0)      // recieve the source code from the client
    {
        free(programFileName);
        errorExit("ERROR :: FILE RECV ERROR");
    }
    n = send(clientSockFD, "I got your code file for grading\n", 33, 0);    // send the confirmation to the client
    if (n < 0)
    {
        free(programFileName);
        errorExit("ERROR :: FILE SEND ERROR");
    }

    // make all necessary files and commands
    char *execFileName = make_exec_filename(reqID);
    char *compileOutputFileName = make_compile_output_filename(reqID);
    char *runtimeOutputFileName = make_runtime_output_filename(reqID);
    char *outputFileName = make_output_filename(reqID);
    char *outputDiffFileName = make_output_diff_filename(reqID);

    char *compileCommand = compile_command(reqID, programFileName, execFileName);
    char *runCommand = run_command(reqID, execFileName);
    char *outputCheckCommand = output_check_command(reqID, outputFileName);

    // Checking if source file successfully compiled or not
    if (system(compileCommand) != 0)
    {
        n = send(clientSockFD, "COMPILER ERROR", 15, 0);    // send feedback to the client
        sleep(1);
        if (n >= 0){
            n=send_file(clientSockFD, compileOutputFileName);   // send the error to the client
        }
            
    }

    // Checking if the source file is successfully execute or not
    else if (system(runCommand) != 0)
    {
        n = send(clientSockFD, "RUNTIME ERROR", 14, 0);     // send feedback to the client
        if (n >= 0)
            n=send_file(clientSockFD, runtimeOutputFileName);   // send the error to the client
    }
    else
    {
        if(system(outputCheckCommand)!=0){                      // checking for output error 
            n = send(clientSockFD, "OUTPUT ERROR", 14, 0);
            if (n >= 0)
                n=send_file(clientSockFD, outputDiffFileName);  // sending the output error to the client
        }
        else{
            n = send(clientSockFD, "PROGRAM RAN", 12, 0);
        }
    }

    // free all the used variables
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

int main(int argc, char *argv[])
{
    // checking if the right inputs are given or not
    if (argc != 2)
    {
        errorExit("ERROR :: No Port Provided");
    }

    // Server and Client socket necessary variables
    int serverSockFD, serverPortNo;
    struct sockaddr_in serverAddr, clientAddr;

    // Make the server socket
    serverSockFD = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSockFD < 0)
        errorExit("ERROR :: Socket Opening Failed");

    // initializing the severAddr
    bzero((char *)&serverAddr, sizeof(serverAddr));
    serverPortNo = atoi(argv[1]);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(serverPortNo);

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

    while (1)
    {
        int clientAddrLen = sizeof(clientAddr);

        // accept the client connection
        int clientSockFD = accept(serverSockFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSockFD < 0)
            errorContinue("ERROR :: Client Socket Accept Failed");

        printf("Accepted Client Connection From :: %s with FD :: %d\n", inet_ntoa(clientAddr.sin_addr), clientSockFD);
        reqID++;

        // calling the grader funciton with the sockfd of the client
        if (grader(clientSockFD) == 0)
        {
            printf("File Grade Successful for Client :: %s\n", inet_ntoa(clientAddr.sin_addr));
        }
        else
        {
            printf("File Grade Unsuccessful for Client :: %s\n", inet_ntoa(clientAddr.sin_addr));
        }
        close(clientSockFD);
        printf("Closed Client Connection From :: %s with FD :: %d\n", inet_ntoa(clientAddr.sin_addr), clientSockFD);
    }

    // finally close the client socket
    close(serverSockFD);

    return 0;
}
