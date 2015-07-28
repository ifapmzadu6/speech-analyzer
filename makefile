
NAME = main
EXTENTION = .out
TARGET = $(NAME)$(EXTENTION)

CXX = clang++
CXXFLAGS = -O2 -Wall
LDFLAGS = -lm
LIBS = -std=c++11 -stdlib=libc++

HEADS := $(wildcard *.h)
SRCS := $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS) $(HEADS)
	$(CXX) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm $(OBJS) $(TARGET)

