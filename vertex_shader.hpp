#pragma once
#include "constants.hpp"

constexpr static const char vertex_shader_source[] = R"(
#version 460 core
#define ROW_LENGTH )" ROW_LENGTH_STR R"(
#define COLUMN_LENGTH )" COLUMN_LENGTH_STR R"(

layout (std430, binding=0) buffer vertex_pos_start {
    vec4 pos [ROW_LENGTH * COLUMN_LENGTH];
} pos_start;
layout (std430, binding=1) buffer vertex_pos_current{
    vec4 pos [ROW_LENGTH * COLUMN_LENGTH];
} pos_current;
layout (std430, binding=2) buffer vertex_pos_next {
    vec4 pos [ROW_LENGTH * COLUMN_LENGTH];
} pos_next;
layout (std430, binding=3) buffer vertex_velocity {
    vec4 data [ROW_LENGTH * COLUMN_LENGTH];
} velocity;

uniform float delta_time = 0.01;
uniform float spinning_speed = 0.5;
uniform float spring_strength = 300;
uniform float gravity_strength = 0.01;
uniform float scaling = 0.01;

uniform bool is_updating = true;

uniform mat4 view_transform;

out vec3 vertex_normal_vec;
out vec3 triangle_normal_vec;
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

    gl_Position = view_transform * vec4(pos_current.pos[vertex_index].xyz, 1);

    if (!is_updating) {
        return;
    }

    if (square_x == vertex_x && (square_y == vertex_y || vertex_y == COLUMN_LENGTH-1)) { // only one vertex processes a place
        if (vertex_y == 0) {
            float alpha = delta_time * spinning_speed;
            float cs = cos(alpha);
            float sn = sin(alpha);

            mat4 rotation_matrix = mat4(
                    cs, 0, -sn, 0,
                    0, 1, 0, 0,
                    sn, 0, cs, 0,
                    0, 0, 0, 1
                    );

            pos_next.pos[vertex_index] = rotation_matrix * pos_current.pos[vertex_index];
        } else {
            velocity.data[vertex_index].y -= gravity_strength * delta_time;

            for (int delta_y = -1; delta_y <= 1; delta_y++) {
                if (vertex_y == COLUMN_LENGTH-1 && delta_y == 1) {
                    break;
                }
                for (int delta_x = -1; delta_x <= 1; delta_x++) {
                    if (delta_x == 0 && delta_y == 0) {
                        continue;
                    }
                    uint other_x = (vertex_x + ROW_LENGTH + delta_x) % ROW_LENGTH;
                    uint other_y = vertex_y + delta_y;
                    uint other_index = other_y * ROW_LENGTH + other_x;

                    float wanted_distance = length (pos_start.pos[vertex_index] - pos_start.pos[other_index]);

                    vec3 position_diff = pos_current.pos[vertex_index].xyz - pos_current.pos[other_index].xyz;

                    float delta_len = wanted_distance - length (position_diff);
                    
                    velocity.data[vertex_index] += vec4 (normalize(position_diff) * delta_len * delta_time * spring_strength, 0);
                }
            }

            velocity.data[vertex_index].y /= 10;


            pos_next.pos[vertex_index] = pos_current.pos[vertex_index] + velocity.data[vertex_index] * delta_time;
        }
    }
}
)";
