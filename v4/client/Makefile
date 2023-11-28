CC = gcc
SRC_DIR = .
UTILS_DIR = utils

# List of source files
CLIENT_SRC = $(SRC_DIR)/client.c
HELPER_SRC = $(UTILS_DIR)/helper.c
FILE_TRANSFER_SRC = $(UTILS_DIR)/file_transfer.c

# List of object files
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
HELPER_OBJ = $(HELPER_SRC:.c=.o)
FILE_TRANSFER_OBJ = $(FILE_TRANSFER_SRC:.c=.o)

# Target: CLIENT
client: $(CLIENT_OBJ) $(HELPER_OBJ) $(FILE_TRANSFER_OBJ)
	$(CC) -o $@ $^

# Rule to compile CLIENT.c
$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CC) -c $< -o $@

# Rule to compile helper.c
$(HELPER_OBJ): $(HELPER_SRC)
	$(CC) -c $< -o $@

# Rule to compile file_transfer.c
$(FILE_TRANSFER_OBJ): $(FILE_TRANSFER_SRC)
	$(CC) -c $< -o $@

# Clean rule
clean:
	rm -f client $(CLIENT_OBJ) $(HELPER_OBJ) $(FILE_TRANSFER_OBJ)
