prog: prog.cpp fragment_shader.hpp vertex_shader.hpp constants.hpp
	g++ prog.cpp -o prog -Wall -lglfw -lGLEW -lGL -std=c++20

.PHONY: format

format:
	clang-format -i *.cpp *.hpp -style=file
