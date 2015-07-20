CXX = g++
CXXFLAGS = -O2 -Wall
LDFLAGS = -lm

main.out: main.cpp voice_wave_analyzer.h
	$(CXX) -o main.out main.cpp

