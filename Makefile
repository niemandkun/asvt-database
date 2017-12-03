BUILD_DIR       := build
SRC_DIR         := src
INCLUDE_DIR     := include

APP             := app
APP_DEPS        := map.o api.o main.o

SERVER          := server
SERVER_DEPS     := socklib.o server.o

CFLAGS          := -Wall -Wextra -Werror -pedantic -std=c99
IFLAGS          := -I./$(INCLUDE_DIR)/
CC              := gcc $(CFLAGS) $(IFLAGS)

ifeq ($(OS),Windows_NT)

LFLAGS          := -lws2_32

endif

all: $(APP) $(SERVER)

$(APP): $(addprefix $(BUILD_DIR)/,$(APP_DEPS))
	$(CC) $^ -o $(APP) $(LFLAGS)

$(SERVER): $(addprefix $(BUILD_DIR)/,$(SERVER_DEPS))
	$(CC) $^ -o $(SERVER) $(LFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR)
	$(CC) -c $< -o $@ $(LFLAGS)

$(BUILD_DIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(APP)
	rm -f $(SERVER)
