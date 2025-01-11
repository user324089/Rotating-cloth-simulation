#pragma once

constexpr static const char fragment_ground_shader_source[] = R"(
#version 460 core

layout (binding=0) uniform sampler2DShadow shadow_sampler;
layout (binding=1) uniform sampler2D shadow_color_sampler;

out vec4 color;
in vec3 light_coords;

uniform uint type = 0;

void main () {

    switch (type) {
        case 0:
            color = texture (shadow_color_sampler, light_coords.xy);
            break;
        case 1:
            float tex = texture (shadow_sampler, light_coords);
            color = vec4 (vec3(0.3,0.3,0.3) * tex + vec3 (0.3,0.3,0.3), 1);
    }
}
)";
