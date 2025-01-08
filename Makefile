prog: prog.cpp fragment_shader.hpp vertex_shader.hpp constants.hpp fragment_shader_ground.hpp vertex_shader_ground.hpp vertex_shader_simple.hpp
	g++ prog.cpp -o prog -Wall -lglfw -lGLEW -lGL -std=c++20

.PHONY: format

format:
	clang-format -i *.cpp *.hpp -style=file
