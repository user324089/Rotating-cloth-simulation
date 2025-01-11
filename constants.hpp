#pragma once
#include <glm/glm.hpp>

#define LIGHT_DIR_STR "vec3(1,0,1)"
constexpr glm::vec3 light_direction(0, 3, 0);

constexpr unsigned int row_length = 60; // remember to also change str
#define ROW_LENGTH_STR "60"
constexpr unsigned int column_length = 40; // remember to also change str
#define COLUMN_LENGTH_STR "40"

constexpr unsigned int vertex_count = column_length * row_length;

constexpr float upper_radius = 0.4f;
constexpr float lower_radius = 0.6f;

constexpr unsigned int shadow_map_size = 2000;
