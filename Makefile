CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

main: main.o render.o scene.o texture.o vec.o lodepng.o
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cc *.hh *.h
	$(CXX) $(CXXFLAGS) $^ -c -o $@
clean:
	rm main *.o
