CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall

OBJS = launcher.o main.o main_tui.o

all: minirpg

minirpg: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o minirpg

launcher.o: launcher.cpp
	$(CXX) $(CXXFLAGS) -c launcher.cpp -o launcher.o

main.o: main.cpp entidades.hpp motor_grafico.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

main_tui.o: main_tui.cpp entidades.hpp motor_tui.hpp
	$(CXX) $(CXXFLAGS) -c main_tui.cpp -o main_tui.o

clean:
	rm -f $(OBJS) minirpg
