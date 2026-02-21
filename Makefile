CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
GTEST_CXXFLAGS = -I$(GTEST_DIR)/include -I$(GTEST_DIR) -pthread

SRC_DIR = src
TEST_SRC_DIR = tests
OBJ_DIR = obj
BIN_DIR = bin

GTEST_DIR = third_party/googletest/googletest

TARGET = $(BIN_DIR)/emulator
TEST_TARGET = $(BIN_DIR)/run_tests

# Exclude main.cpp from the common objects used by tests
COMMON_SRCS = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
COMMON_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(COMMON_SRCS))

# Objects for the main emulator executable
MAIN_OBJ = $(OBJ_DIR)/main.o

TEST_SRCS = $(wildcard $(TEST_SRC_DIR)/*.cpp)
TEST_OBJS = $(patsubst $(TEST_SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(TEST_SRCS))

# Use gtest_main to avoid having to write our own main() function for tests
GTEST_SRCS = $(GTEST_DIR)/src/gtest_main.cc $(GTEST_DIR)/src/gtest-all.cc
GTEST_OBJS = $(patsubst $(GTEST_DIR)/src/%.cc, $(OBJ_DIR)/%.o, $(GTEST_SRCS))

.PHONY: all test clean

all: $(TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TARGET): $(MAIN_OBJ) $(COMMON_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Link only test-related objects together to create the test runner
$(TEST_TARGET): $(COMMON_OBJS) $(TEST_OBJS) $(GTEST_OBJS) | $(BIN_DIR)
	$(CXX) $(GTEST_CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c -o $@ $<

# A specific rule for compiling gtest source files
$(OBJ_DIR)/gtest-all.o: $(GTEST_DIR)/src/gtest-all.cc | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/gtest_main.o: $(GTEST_DIR)/src/gtest_main.cc | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c -o $@ $<

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
