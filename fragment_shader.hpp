#pragma once
#include "constants.hpp"

constexpr static const char fragment_shader_source[] = R"(
#version 460 core

out vec4 color;
in vec3 vertex_normal_vec;
in vec2 tex_coord;

vec3 light_dir = normalize()" LIGHT_DIR_STR R"();

uniform float alpha = 0.6;

void main () {
    float light_intensity = min(max(dot(vertex_normal_vec,light_dir), 0)*0.5 + 0.4, 1);
    color = vec4 (light_intensity * tex_coord, 0, alpha);
}
)";
