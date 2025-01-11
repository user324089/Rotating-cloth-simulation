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

out vec3 vertex_normal_vec;
out vec2 tex_coord;

void main () {
    uint square_index = gl_VertexID / 6;

    uint square_y = square_index / ROW_LENGTH;
    uint square_x = square_index % ROW_LENGTH;

    uint x_offsets [6] = {0,0,1,1,0,1}; // offsets within the square of vertices
    uint y_offsets [6] = {0,1,0,0,1,1};

    uint vertex_x = (square_x + x_offsets[gl_VertexID % 6]) % ROW_LENGTH;
    uint vertex_y = square_y + y_offsets[gl_VertexID % 6];

    tex_coord = vec2(float(square_x + x_offsets[gl_VertexID % 6]) / ROW_LENGTH,
            float(vertex_y)/COLUMN_LENGTH);

    uint vertex_index = vertex_y * ROW_LENGTH + vertex_x;

    gl_Position = light_transform * vec4(pos_current.pos[vertex_index].xyz, 1);

    vec3 normal_vec = vec3 (0,0,0);
    uint adjacent_triangles = 0;

    vec3 my_position = pos_current.pos[vertex_index].xyz;

    uint left_x = (vertex_x + ROW_LENGTH - 1) % ROW_LENGTH;
    uint right_x = (vertex_x + 1) % ROW_LENGTH;

    if (vertex_y > 0) {
        vec3 vectors_to_adjacents [] = {
            pos_current.pos[vertex_y * ROW_LENGTH + left_x].xyz - my_position,
            pos_current.pos[(vertex_y-1) * ROW_LENGTH + vertex_x].xyz - my_position,
            pos_current.pos[vertex_y * ROW_LENGTH + right_x].xyz - my_position,
        };
        for (uint i = 0; i < 2; i++) {
            normal_vec += normalize (cross (vectors_to_adjacents[i+1], vectors_to_adjacents[i]));
        }
        adjacent_triangles += 2;
    }
    if (vertex_y < COLUMN_LENGTH-1) {
        vec3 vectors_to_adjacents [] = {
            pos_current.pos[vertex_y * ROW_LENGTH + left_x].xyz - my_position,
            pos_current.pos[(vertex_y+1) * ROW_LENGTH + vertex_x].xyz - my_position,
            pos_current.pos[vertex_y * ROW_LENGTH + right_x].xyz - my_position,
        };
        for (uint i = 0; i < 2; i++) {
            normal_vec += normalize (cross (vectors_to_adjacents[i], vectors_to_adjacents[i+1]));
        }
        adjacent_triangles += 2;
    }

    vertex_normal_vec = normal_vec / adjacent_triangles;
}
)";
