.PHONY: all clean run

all: clean main run 

CXX = clang++
CXXFLAGS = -std=c++23 -O2 -Wall -Wextra
SOURCES = main.cpp

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    INC = -I/opt/homebrew/include
    LDFLAGS = /opt/homebrew/lib/libraylib.a -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif

ifeq ($(UNAME_S), Linux)
    INC = 
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

main: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o main $(INC) $(LDFLAGS)

run:
	./main

clean:
	rm -f main
