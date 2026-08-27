CXX ?= g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude

BUILD_DIR := build
SRC := src/Delays.cpp
HEADERS := include/Effects_V2.h include/AudioFile.h

.PHONY: all clean

all: $(BUILD_DIR)/delays

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/delays: $(SRC) $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@

clean:
	rm -rf $(BUILD_DIR)