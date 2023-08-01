CC=g++
FLAGS=-std=c++17 -Wall -Wextra

main: main.cc render.cc scene.cc vec.cc lodepng.cc
	$(CC) $(FLAGS) -o $@ $^
