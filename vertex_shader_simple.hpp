#pragma once
#include "constants.hpp"

constexpr static const char vertex_simple_shader_source[] = R"(
#version 460 core
#define ROW_LENGTH )" ROW_LENGTH_STR R"(
#define COLUMN_LENGTH )" COLUMN_LENGTH_STR R"(

layout (std430, binding=1) buffer vertex_pos_current{
    vec4 pos [ROW_LENGTH * COLUMN_LENGTH];
} pos_current;

uniform mat4 light_transform;

void main () {
    uint square_index = gl_VertexID / 6;

    uint square_y = square_index / ROW_LENGTH;
    uint square_x = square_index % ROW_LENGTH;

    uint x_offsets [6] = {0,0,1,1,0,1}; // offsets within the square of vertices
    uint y_offsets [6] = {0,1,0,0,1,1};

    uint vertex_x = (square_x + x_offsets[gl_VertexID % 6]) % ROW_LENGTH;
    uint vertex_y = square_y + y_offsets[gl_VertexID % 6];

    uint vertex_index = vertex_y * ROW_LENGTH + vertex_x;

    gl_Position = light_transform * vec4(pos_current.pos[vertex_index].xyz, 1);
}
)";
