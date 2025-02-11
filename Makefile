prog: prog.o painter.o
	g++ prog.o painter.o -o prog  -lglfw -lGLEW -lGL -std=c++20

prog.o: prog.cpp Painter.hpp constants.hpp
	g++ prog.cpp -o prog.o -Wall -std=c++20 -c

painter.o: Painter.cpp Painter.hpp constants.hpp fragment_shader.hpp vertex_shader.hpp constants.hpp fragment_shader_ground.hpp vertex_shader_ground.hpp 
	g++ Painter.cpp -o painter.o -Wall -std=c++20 -c

.PHONY: format clean

clean:
	rm -rf prog.o painter.o prog

format:
	clang-format -i *.cpp *.hpp -style=file
