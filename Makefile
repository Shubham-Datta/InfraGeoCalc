# Makefile for InfraGeoCalc project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Iinclude  # Warnings, C99 standard, optimization, include headers
LDFLAGS = -lm -pthread  # Link math and pthread libraries

# Directories
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source files for main executable
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/geometry.c $(SRC_DIR)/io.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Test object files (reuse geometry.o and io.o, compile test-specific)
TEST_OBJS = $(BUILD_DIR)/test_main.o $(BUILD_DIR)/test_geometry.o $(BUILD_DIR)/geometry.o $(BUILD_DIR)/io.o

# Targets
all: $(BUILD_DIR)/infrageocalc

$(BUILD_DIR)/infrageocalc: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(BUILD_DIR)/test_infrageocalc
	./$(BUILD_DIR)/test_infrageocalc

$(BUILD_DIR)/test_infrageocalc: $(TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean