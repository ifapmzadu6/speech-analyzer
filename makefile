

TARGET = main

ifeq ($(OS), Windows_NT)
	TARGET = main.exe
endif


CXX = clang++
CXXFLAGS = -O2 -Wall
LDFLAGS = -lm
LIBS = -std=c++11 -stdlib=libc++

HEADS = $(shell ls *.h)
SRCS = $(shell ls *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS) $(HEADS)
	$(CXX) -o main.out main.cpp wave.cpp $(LIBS)

clean:
	rm $(OBJS) $(TARGET)

