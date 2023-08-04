CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3

main: main.o render.o scene.o texture.o vec.o lodepng.o
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cc %.hh lodepng.hh
	$(CXX) $(CXXFLAGS) $(filter-out %.hh, $^) -c -o $@
clean:
	rm main *.o
