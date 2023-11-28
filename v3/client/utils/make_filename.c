#include "make_filename.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>

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

