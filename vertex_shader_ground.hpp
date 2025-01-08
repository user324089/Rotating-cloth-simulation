#pragma once

constexpr static const char vertex_ground_shader_source[] = R"(
#version 460 core

layout (location=0) in vec3 position;

uniform mat4 view_transform;
uniform mat4 light_transform;

out vec3 light_coords;

out vec2 tex_coords;

void main () {
    vec4 light_coords_4 = light_transform * vec4(position, 1);
    light_coords = light_coords_4.xyz / light_coords_4.w;
    light_coords = light_coords / 2 + vec3(0.5,0.5,0.5);
    gl_Position = view_transform * vec4(position, 1);
    tex_coords = position.xz / 4 + vec2(0.5,0.5);
}
)";
