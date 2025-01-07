#pragma once

constexpr static const char vertex_ground_shader_source[] = R"(
#version 460 core

layout (location=0) in vec3 position;

uniform mat4 view_transform;
uniform mat4 light_transform;

void main () {
    gl_Position = view_transform * vec4(position, 1);
}
)";
