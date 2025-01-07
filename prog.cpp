#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <numbers>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


constexpr unsigned int row_length = 60; // remember to also change str
#define ROW_LENGTH_STR "60"
constexpr unsigned int column_length = 40; // remember to also change str
#define COLUMN_LENGTH_STR "40"

constexpr unsigned int vertex_count = column_length * row_length;
constexpr float upper_radius = 0.4f;
constexpr float lower_radius = 0.6f;

constexpr static const char vertex_shader_source [] = R"(
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

constexpr static const char fragment_shader_source [] = R"(
#version 460 core

out vec4 color;
in vec3 vertex_normal_vec;
in vec2 tex_coord;

vec3 light_dir = normalize(vec3(1,0,1));

void main () {
    float light_intensity = min(max(dot(vertex_normal_vec,light_dir), 0)*0.5 + 0.4, 1);
    color = vec4 (light_intensity * tex_coord, 0, 1);
}
)";


GLuint create_shader (GLint type, const char * source, const std::string &shader_name_str) {
    GLuint shader = glCreateShader (type);
    glShaderSource (shader, 1, &source, nullptr);
    glCompileShader (shader);
    GLint success;
    glGetShaderiv (shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char buffer [512];
        glGetShaderInfoLog (shader, 512, nullptr, buffer);
        throw std::runtime_error (shader_name_str + buffer);
    }
    return shader;
};


int main () {

    glfwInit ();
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow (100, 100, "title", nullptr, nullptr);
    glfwMakeContextCurrent (window);

    glewExperimental = GL_TRUE;
    glewInit ();

    GLuint VAO;
    glGenVertexArrays (1, &VAO);
    glBindVertexArray (VAO);

    GLuint v_shader = create_shader (GL_VERTEX_SHADER, vertex_shader_source, "vertex shader");
    GLuint f_shader = create_shader (GL_FRAGMENT_SHADER, fragment_shader_source, "fragment shader");

    glEnable (GL_DEPTH_TEST);

    GLuint buffers [4];
    glGenBuffers (4, buffers);

    float vertex_positions[vertex_count][4];
    float zeros[vertex_count][4];
    std::memset (zeros, 0, sizeof(zeros));

    for (unsigned int y = 0; y < column_length; y++) {
        for (unsigned int x = 0; x < row_length; x++) {
            float y_fraction = static_cast<float>(y)/column_length;
            float radius = (1-y_fraction)*upper_radius + y_fraction * lower_radius;
            vertex_positions[y*row_length + x][0] = std::cos
                (std::numbers::pi_v<float> * 2 * static_cast<float>(x) /
                 row_length) * radius;
            vertex_positions[y*row_length + x][1] = 0.5f - y_fraction;
            vertex_positions[y*row_length + x][2] = -std::sin
                (std::numbers::pi_v<float> * 2 * static_cast<float>(x) /
                 row_length) * radius;
            
        }
    }

    glBindBuffer (GL_SHADER_STORAGE_BUFFER, buffers[0]);
    glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
    glBindBuffer (GL_SHADER_STORAGE_BUFFER, buffers[1]);
    glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
    glBindBuffer (GL_SHADER_STORAGE_BUFFER, buffers[2]);
    glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), nullptr, GL_STATIC_DRAW);
    glBindBuffer (GL_SHADER_STORAGE_BUFFER, buffers[3]);
    glBufferData (GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), zeros, GL_STATIC_DRAW);
    glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
    unsigned int current_buffer = 0;

    glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 0, buffers[0]);

    GLuint program = glCreateProgram ();
    glAttachShader (program, v_shader);
    glAttachShader (program, f_shader);
    glLinkProgram (program);

    {
        int success;
        glGetProgramiv (program, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log_buffer [512];
            glGetProgramInfoLog (program, 512,nullptr, info_log_buffer);
            std::cout << "error linking program: " << info_log_buffer << "\n";
        }
    }

    glm::mat4 view_transform (1.0f);
    view_transform = glm::translate (view_transform, glm::vec3 (0,0,-3));
    view_transform = glm::rotate (view_transform, 0.4f, glm::vec3 (1,0,0));
    view_transform = glm::perspective (45.0f, 800.0f/600.0f, 0.1f, 100.0f) * view_transform;
    GLuint view_transform_uniform_location = glGetUniformLocation (program, "view_transform");
    glProgramUniformMatrix4fv (program, view_transform_uniform_location, 1, GL_FALSE, glm::value_ptr(view_transform));


    GLint delta_time_uniform_location = glGetUniformLocation (program, "delta_time");

    float time_passed = 0;
    auto prev_time_point = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose (window)) {
        auto cur_time_point = std::chrono::high_resolution_clock::now();
        float elapsed_seconds = std::chrono::duration<float> {cur_time_point - prev_time_point}.count();
        prev_time_point = cur_time_point;
        glUniform1f (delta_time_uniform_location, elapsed_seconds);
        time_passed += elapsed_seconds;


        int width, height;
        glfwGetWindowSize (window, &width, &height);
        glViewport (0, 0, width, height);

        glClearColor (1, 0, 1, 1);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram (program);
        glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 1, buffers[1+current_buffer]);
        current_buffer ^= 1;
        glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 2, buffers[1+current_buffer]);
        glBindBufferBase (GL_SHADER_STORAGE_BUFFER, 3, buffers[3]);

        glDrawArrays(GL_TRIANGLES, 0, 6*row_length*(column_length-1));

        glFinish();

        glfwSwapBuffers (window);
        glfwPollEvents ();
    }

    glfwDestroyWindow (window);
}
