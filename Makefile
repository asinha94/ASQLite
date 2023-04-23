
PREFIX    = $(shell pwd)

CPP      := clang++
WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
	        -Wwrite-strings -Wcast-qual -Wconversion -Wno-long-long \
	        -Wredundant-decls -Winline  -Wno-sign-conversion -Wno-conversion\

CPPFLAGS := -g $(WARNINGS) -std=c++17 -fno-exceptions $(INCLUDES)


CPP_SRC   := $(shell find $(PREFIX)/ -name *.cpp)
CPP_OBJS  := $(CPP_SRC:%.cpp=%.o)
BIN       := asqlite

.PHONY: clean
.SUFFIXES: .o .cpp

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@ $(CPPFLAGS)

build: $(CPP_OBJS)
	$(CPP) $(CPPFLAGS) $(CPP_OBJS) -o $(BIN)

clean:
	@rm -rf $(BIN) $(CPP_OBJS) 