#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE_BYTES 4
#define MAX_CLIENTS 10

#define errorExit(msg) \
    {                  \
        perror(msg);   \
        return -1;     \
    }

#define error(msg)   \
    {                \
        perror(msg); \
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


int recv_file(int sockfd, char *file_path);
int send_file(int sockfd, char *file_path);
char *compile_command(int id, char *programFile, char *execFile);
char *run_command(int id, char *execFile);
char *output_check_command(int id, char *outputFile);
char *make_program_filename(int id);
char *make_exec_filename(int id);
char *make_compile_output_filename(int id);
char *make_runtime_output_filename(int id);
char *make_output_filename(int id);
char *make_output_diff_filename(int id);
