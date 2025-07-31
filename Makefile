# Made based on (plagiarism lol): 
# https://spin.atomicobject.com/makefile-c-projects/

TARGET := narcissus
BUILD_DIR := ./build

SRC_DIRS := ./src  
INC_DIRS := ./include ./include/imgui ./include/imgui/backends

IMGUI_DIR := ./include/imgui
IMGUI_SRC := $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/backends/imgui_impl_sdlrenderer3.cpp $(IMGUI_DIR)/backends/imgui_impl_sdl3.cpp 

SRCS := $(IMGUI_SRC) $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

UNAME := $(shell uname)

CC := clang++
CPPFLAGS := -g -Wall -Wextra -MMD -MP $(INC_FLAGS) $(shell pkg-config --cflags opencv4) $(shell pkg-config --cflags SDL3)
LDFLAGS := $(shell pkg-config --libs SDL3) $(shell pkg-config --libs opencv4)


all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS) 
	$(CC) $(OBJS) -o $@ $(LDFLAGS) 

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@) 
	$(CC) $(CPPFLAGS) -c $< -o $@

.PHONY: clean run
run: 
	pushd build && ./narcissus && popd

clean:
	rm -rf build/*

-include $(DEPS)
