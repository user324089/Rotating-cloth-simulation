#pragma once

constexpr static const char fragment_shader_source[] = R"(
#version 460 core

out vec4 color;
in vec3 vertex_normal_vec;
in vec2 tex_coord;

uniform vec3 light_dir;

uniform float alpha = 0.6;

void main () {
    float light_intensity = min(max(dot(vertex_normal_vec,normalize(light_dir)), 0)*0.5 + 0.4, 1);
    color = vec4 (light_intensity * tex_coord, 0, alpha);
}
)";
