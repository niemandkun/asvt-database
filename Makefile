BUILD_DIR       := build
SRC_DIR         := src
INCLUDE_DIR     := include

CLIENT          := client
CLIENT_DEPS     := socklib.o api.o utils.o client.o

SERVER          := server
SERVER_DEPS     := socklib.o server.o map.o api.o

CFLAGS          := -Wall -Wextra -Werror -pedantic -std=c99
IFLAGS          := -I./$(INCLUDE_DIR)/
CC              := gcc $(CFLAGS) $(IFLAGS)

ifeq ($(OS),Windows_NT)

LFLAGS          := -lws2_32

endif

all: $(CLIENT) $(SERVER)

$(CLIENT): $(addprefix $(BUILD_DIR)/,$(CLIENT_DEPS))
	$(CC) $^ -o $(CLIENT) $(LFLAGS)

$(SERVER): $(addprefix $(BUILD_DIR)/,$(SERVER_DEPS))
	$(CC) $^ -o $(SERVER) $(LFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR)
	$(CC) -c $< -o $@ $(LFLAGS)

$(BUILD_DIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(CLIENT)
	rm -f $(SERVER)
