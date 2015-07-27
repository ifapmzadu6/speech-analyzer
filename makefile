

TARGET = main


ifeq (\$(OS),WINDOWS_NT)
	EXTENTION = ".exe"
else
	EXTENTION = ".out"
endif


CXX = clang++
CXXFLAGS = -O2 -Wall
LDFLAGS = -lm
LIBS = -std=c++11 -stdlib=libc++

HEADS := $(wildcard *.h)
SRCS := $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

$(TARGET)$(EXTENTION): $(OBJS) $(HEADS)
	$(CXX) -o $(TARGET)$(EXTENTION) $(SRCS) $(LIBS)

clean:
	rm $(OBJS) $(TARGET)$(EXTENTION)

