# Made based on (plagiarism lol): 
# https://spin.atomicobject.com/makefile-c-projects/

TARGET := narcissus
BUILD_DIR := ./build

SRC_DIRS := ./src  
INC_DIRS := ./src/ext/imgui ./src/ext/imgui/backends ./src/ext/SDL3


#SRCS := $(IMGUI_SRC) $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

UNAME := $(shell uname)

CC := clang++
CPPFLAGS := -DPLATFORM_MAC -g -Wall -Wextra $(INC_FLAGS) $(shell pkg-config --cflags opencv4) $(shell pkg-config --cflags SDL3) $(shell pkg-config --cflags freetype2)
LDFLAGS := $(shell pkg-config --libs SDL3) $(shell pkg-config --libs opencv4) $(shell pkg-config --libs freetype2)


all:
	$(CC) $(CPPFLAGS) ./src/narcissus.cpp $(LDFLAGS) -o $(BUILD_DIR)/$(TARGET)

.PHONY: clean run
run: 
	pushd build && ./narcissus && popd

clean:
	rm -rf build/*

-include $(DEPS)
