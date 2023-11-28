#include "file_update.h"
#include "helper.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>

// Function to append request ID and status to a status file
int writeStatusToFile(int requestID, char *status)
{
    // Open the file in append mode
    FILE *file = fopen("request_status.csv", "a");
    if (file == NULL)
    {
        errorExit("Error opening file");
    }
    // Append request ID and status to the file
    fprintf(file, "%d,%s\n", requestID, status);
    // Close the file
    fclose(file);
    return 0;
}

// Function to update status in a status file
int updateStatusToFile(int requestID, char *newStatus)
{
    // Open the file in read mode
    FILE *file = fopen("request_status.csv", "r");
    if (file == NULL)
    {
        errorExit("Error opening file");
    }

    // Create a temporary file to write updated content
    FILE *tempFile = fopen("temp_status.csv", "w");
    if (tempFile == NULL)
    {
        fclose(file);
        errorExit("Error opening temp file");
    }

    // // Search for the request ID in the file and update the status
    char line[256]; // Adjust the size as needed
    int requestFound = 0;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // printf("%s\n", line);
        char *token = strtok(line, ",");
        // printf("%s\n", token);
        char s1[11];
        sprintf(s1, "%d", requestID);
        if (token != NULL && strcmp(token, s1) == 0)
        {
            // Found the request ID, update the status
            fprintf(tempFile, "%s,%s\n", s1, newStatus);
            requestFound = 1;
        }
        else
        {
            token = strtok(NULL, " ");
            // Copy the line as is to the temporary file
            fprintf(tempFile, "%s,%s", line, token);
        }
    }

    // Close the files
    fclose(file);
    fclose(tempFile);

    // Remove the original file
    if (remove("request_status.csv") != 0)
    {
        errorExit("Error removing original file");
    }

    // Rename the temporary file to the original file
    if (rename("temp_status.csv", "request_status.csv") != 0)
    {
        errorExit("Error renaming temp file");
    }
    if (requestFound)
    {
        return 0; // Update successful
    }
    else
    {
        return -1; // Request ID not found
    }
}

// Function to read status from a status file
char *readStatusFromFile(int requestID)
{
    // Open the file in read mode
    FILE *file = fopen("request_status.csv", "r");
    if (file == NULL)
    {
        error("Error opening file");
        return NULL;
    }

    // Search for the request ID in the file
    char line[256]; // Adjust the size as needed
    while (fgets(line, sizeof(line), file) != NULL)
    {
        char *token = strtok(line, ",");
        char s1[11];
        sprintf(s1, "%d", requestID);
        if (token != NULL && strcmp(token, s1) == 0)
        {
            // Found the request ID, retrieve the status
            token = strtok(NULL, ",");
            char *status = strdup(token); // Dynamically allocate memory for the status
            status[strcspn(status, "\n")] = '\0';
            fclose(file);
            return status; // Return the dynamically allocated status
        }
    }

    // Request ID not found
    fclose(file);
    return NULL; // Return NULL to indicate not found
}

// Function to read status from a status file
char *readRemarksFromFile(char *statusID)
{
    // Open the file in read mode
    FILE *file = fopen("remarks.txt", "r");
    if (file == NULL)
    {
        error("Error opening file");
        return NULL;
    }

    // Search for the request ID in the file
    char line[256]; // Adjust the size as needed
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // printf("Status :: %s", statusID);
        char *token = strtok(line, ",");
        // printf("Token 1 :: %s\n", token);
        if (token != NULL && (strcmp(token, statusID) == 0))
        {
            // Found the request ID, retrieve the status
            token = strtok(NULL, ",");
            // printf("Token 2 :: %s", token);
            char *remarks = strdup(token); // Dynamically allocate memory for the status
            // remarks[strcspn(remarks, "\n")] = '\0';
            fclose(file);
            return remarks; // Return the dynamically allocated status
        }
    }

    // Request ID not found
    fclose(file);
    return NULL; // Return NULL to indicate not found
}
