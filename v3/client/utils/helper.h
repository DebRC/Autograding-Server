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

