CXX = g++
CXXFLAGS = -O2 -Wall
LDFLAGS = -lm

main.out: main.cpp voice_wave_analyzer.h wave.h wave.cpp kmeans_method.h gnuplot.h
	$(CXX) -o main.out main.cpp wave.cpp

