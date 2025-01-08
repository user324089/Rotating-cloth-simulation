#pragma once

constexpr static const char fragment_ground_shader_source[] = R"(
#version 460 core

//layout (binding=0) uniform sampler2DShadow shadow_sampler;
layout (binding=0) uniform sampler2D shadow_sampler;

out vec4 color;
in vec3 light_coords;
in vec2 tex_coords;

void main () {
    //float red = textureProj (shadow_sampler, light_coords.xyw);
    //color = vec4 (red, 0.4, 0.4, 1);
    //color = vec4 (tex_coords, 0, 1);
    /*
    if (light_coords.x > 1 || light_coords.x < -1) {
        color = vec4 (1,1,1,1);
    } else {
        color = vec4 (light_coords.xy, 0, 1);
    }
    */
    //color = texture (shadow_sampler, light_coords.xz);
    //color = textureProj (shadow_sampler, light_coords.xzw);
    color = texture (shadow_sampler, tex_coords);
}
)";
