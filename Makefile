CC := gcc
CFLAGS := -Iinclude -Wall
LDFLAGS := -lsystemd -lblkid
SRC_DIR := src
BUILD_DIR := build
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET := $(BUILD_DIR)/cubby
DEBUG_TARGET := $(BUILD_DIR)/cubby-debug

.PHONY: all clean debug

all: $(BUILD_DIR) $(TARGET)

debug: CFLAGS += -g
debug: $(BUILD_DIR) $(DEBUG_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(DEBUG_TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

