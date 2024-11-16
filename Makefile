CXX = g++
CXXFLAGS = -std=c++20 -I./include/ -Wall -Wextra -pedantic
LDFLAGS = -lglfw
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/main

EXCLUDE_FILES = $(SRC_DIR)/dummyfile.cpp
SRCS = main.cpp $(SRC_DIR)/glad.c
OBJS = $(BUILD_DIR)/glad.o $(BUILD_DIR)/main.o

$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	gcc -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	@$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: run clean
