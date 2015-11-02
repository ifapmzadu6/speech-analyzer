
NAME = main
EXTENTION = .out
TARGET = $(NAME)$(EXTENTION)

INCLUDES = -I/usr/local/include
LIBS = -L/usr/local/lib

CXX = clang++
CXXFLAGS = -O2 -Wall -std=c++11 $(INCLUDES)
LDFLAGS = -lm -lfftw3
CFLAGS = -c -Wall 

HEADS := $(wildcard *.h)
SRCS := $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS) $(HEADS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm $(OBJS) $(TARGET)

