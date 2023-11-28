// Function to append request ID and status to a status file
int writeStatusToFile(int requestID, char *status);

// Function to update status in a status file
int updateStatusToFile(int requestID, char *newStatus);

// Function to read status from a status file
char *readStatusFromFile(int requestID);

// Function to read status from a status file
char *readRemarksFromFile(char *statusID);