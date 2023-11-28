CC = gcc
SRC_DIR = .
UTILS_DIR = utils

# List of source files
CLIENT_SRC = $(SRC_DIR)/client.c
HELPER_SRC = $(UTILS_DIR)/helper.c
CIRCULAR_QUEUE_SRC = $(UTILS_DIR)/circular_queue.c
MAKE_FILENAME_SRC = $(UTILS_DIR)/make_filename.c
SYSTEM_COMMANDS_SRC = $(UTILS_DIR)/system_commands.c
FILE_TRANSFER_SRC = $(UTILS_DIR)/file_transfer.c

# List of object files
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
HELPER_OBJ = $(HELPER_SRC:.c=.o)
CIRCULAR_QUEUE_OBJ = $(CIRCULAR_QUEUE_SRC:.c=.o)
MAKE_FILENAME_OBJ = $(MAKE_FILENAME_SRC:.c=.o)
SYSTEM_COMMANDS_OBJ = $(SYSTEM_COMMANDS_SRC:.c=.o)
FILE_TRANSFER_OBJ = $(FILE_TRANSFER_SRC:.c=.o)

# Target: CLIENT
client: $(CLIENT_OBJ) $(HELPER_OBJ) $(CIRCULAR_QUEUE_OBJ) $(MAKE_FILENAME_OBJ) $(SYSTEM_COMMANDS_OBJ) $(FILE_TRANSFER_OBJ)
	$(CC) -o $@ $^

# Rule to compile CLIENT.c
$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CC) -c $< -o $@

# Rule to compile helper.c
$(HELPER_OBJ): $(HELPER_SRC)
	$(CC) -c $< -o $@

# Rule to compile circular_queue.c
$(CIRCULAR_QUEUE_OBJ): $(CIRCULAR_QUEUE_SRC)
	$(CC) -c $< -o $@

# Rule to compile make_filename.c
$(MAKE_FILENAME_OBJ): $(MAKE_FILENAME_SRC)
	$(CC) -c $< -o $@

# Rule to compile system_commands.c
$(SYSTEM_COMMANDS_OBJ): $(SYSTEM_COMMANDS_SRC)
	$(CC) -c $< -o $@

# Rule to compile file_transfer.c
$(FILE_TRANSFER_OBJ): $(FILE_TRANSFER_SRC)
	$(CC) -c $< -o $@

# Clean rule
clean:
	rm -f client $(CLIENT_OBJ) $(HELPER_OBJ) $(CIRCULAR_QUEUE_OBJ) $(MAKE_FILENAME_OBJ) $(SYSTEM_COMMANDS_OBJ) $(FILE_TRANSFER_OBJ)
