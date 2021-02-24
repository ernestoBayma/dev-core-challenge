CC := gcc

S_TARGET_NAME := dev-core-server
C_TARGET_NAME := dev-core-client

BUILD_DIR ?= ./build
SRC_DIR ?= ./src
SRC_DIR_SERVER ?= $(SRC_DIR)/server
SRC_DIR_CLIENT ?= $(SRC_DIR)/client

INCLUDE_DIR ?= /usr/local/include ./include

SRCS_SERVER := $(shell find $(SRC_DIR_SERVER) -name '*.c')
SRCS_CLIENT := $(shell find $(SRC_DIR_CLIENT) -name '*.c')
SRCS := $(shell find $(SRC_DIR) -name '*.c')

OBJ_SERVER := $(SRCS_SERVER:%=$(BUILD_DIR)/%.o)
OBJ_CLIENT := $(SRCS_CLIENT:%=$(BUILD_DIR)/%.o)
OBJ_COMMON := $(filter-out $(OBJ_CLIENT) $(OBJ_SERVER), $(SRCS:%=$(BUILD_DIR)/%.o))

DEPS := $(OBJ_CLIENT:.o=.d) $(OBJ_SERVER:.o=.d) $(OBJ_COMMON:.o=.d)

CFLAGS := $(addprefix -I,$(INCLUDE_DIR)) -Wall -g -MMD -std=c99

CLIENT_PATH := $(BUILD_DIR)/$(C_TARGET_NAME)
SERVER_PATH := $(BUILD_DIR)/$(S_TARGET_NAME)


build:  $(CLIENT_PATH) $(SERVER_PATH) 
	@echo "finalizado"

$(CLIENT_PATH): $(OBJ_CLIENT) $(OBJ_COMMON)
	@@$(CC) $(OBJ_CLIENT) $(OBJ_COMMON) -o $@ 

$(SERVER_PATH): $(OBJ_SERVER) $(OBJ_COMMON)
	@@$(CC) $(OBJ_SERVER) $(OBJ_COMMON) -o $@ 


$(BUILD_DIR)/%.c.o : %.c 
	$(MKDIR_P) $(dir $@) 
	@@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Deletando pasta ./build"
	@$(RM) -r $(BUILD_DIR)


.PHONY: clean

-include $(DEPS)

MKDIR_P ?= mkdir -p