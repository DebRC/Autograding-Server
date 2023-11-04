#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE_BYTES 4
#define MAX_CLIENTS 1

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

int grader(int clientSockFD)
{
    int n;
    char *programFileName = make_program_filename(clientSockFD);
    if (recv_file(clientSockFD, programFileName) != 0)
    {
        free(programFileName);
        errorExit("ERROR :: FILE RECV ERROR");
    }
    n = send(clientSockFD, "I got your code file for grading\n", 33, 0);
    if (n < 0)
    {
        free(programFileName);
        errorExit("ERROR :: FILE SEND ERROR");
    }

    char *execFileName = make_exec_filename(clientSockFD);
    char *compileOutputFileName = make_compile_output_filename(clientSockFD);
    char *runtimeOutputFileName = make_runtime_output_filename(clientSockFD);
    char *outputFileName = make_output_filename(clientSockFD);
    char *outputDiffFileName = make_output_diff_filename(clientSockFD);

    char *compileCommand = compile_command(clientSockFD, programFileName, execFileName);
    char *runCommand = run_command(clientSockFD, execFileName);
    char *outputCheckCommand = output_check_command(clientSockFD, outputFileName);

    if (system(compileCommand) != 0)
    {
        n = send(clientSockFD, "COMPILER ERROR", 15, 0);
        if (n >= 0)
            n=send_file(clientSockFD, compileOutputFileName);
    }
    else if (system(runCommand) != 0)
    {
        n = send(clientSockFD, "RUNTIME ERROR", 14, 0);
        if (n >= 0)
            n=send_file(clientSockFD, runtimeOutputFileName);
    }
    else
    {
        if(system(outputCheckCommand)!=0){
            n = send(clientSockFD, "OUTPUT ERROR", 14, 0);
            if (n >= 0)
                n=send_file(clientSockFD, outputDiffFileName);
        }
        else{
            n = send(clientSockFD, "PROGRAM RAN", 12, 0);
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

int main(int argc, char *argv[])
{
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
    if (listen(serverSockFD, 5) < 0)
    {
        errorExit("ERROR :: Socket Listening Failed");
    }

    while (1)
    {
        int clientAddrLen = sizeof(clientAddr);
        int clientSockFD = accept(serverSockFD, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSockFD < 0)
            errorContinue("ERROR :: Client Socket Accept Failed");
        printf("Accepted Client Connection From :: %s with FD :: %d\n", inet_ntoa(clientAddr.sin_addr), clientSockFD);
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

    close(serverSockFD);

    return 0;
}