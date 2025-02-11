#pragma once
#include <glm/glm.hpp>

constexpr glm::vec3 light_dir(0.5,3,0.5);

constexpr unsigned int row_length = 60; // remember to also change str
#define ROW_LENGTH_STR "60"
constexpr unsigned int column_length = 40; // remember to also change str
#define COLUMN_LENGTH_STR "40"

constexpr unsigned int vertex_count = column_length * row_length;

constexpr float upper_radius = 0.4f;
constexpr float lower_radius = 0.6f;

constexpr unsigned int shadow_map_size = 2000;
