#pragma once

constexpr static const char fragment_ground_shader_source[] = R"(
#version 460 core

out vec4 color;

void main () {
    color = vec4 (1, 1, 1, 1);
}
)";
