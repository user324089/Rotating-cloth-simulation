#include "constants.hpp"
#include "fragment_shader.hpp"
#include "vertex_shader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <numbers>
#include <sstream>
#include <stdexcept>

GLuint create_shader(GLint type, const char *source, const std::string &shader_name_str) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log_buffer[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log_buffer);
        std::stringstream stream;
        stream << "error linking program " << shader_name_str << ": " << info_log_buffer;
        throw std::runtime_error(stream.str());
    }
    return shader;
}

void link_program(GLuint program, const std::string &program_name_str) {

    glLinkProgram(program);

    {
        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char info_log_buffer[512];
            glGetProgramInfoLog(program, 512, nullptr, info_log_buffer);
            std::stringstream stream;
            stream << "error linking program " << program_name_str << ": " << info_log_buffer;
            throw std::runtime_error(stream.str());
        }
    }
}

class Painter {
    private:
        GLFWwindow *window = nullptr;
        GLint delta_time_uniform_location = 0;
        GLuint program = 0;
        unsigned int current_buffer = 0;
        GLuint buffers[4];

        void init_buffers() {}

    public:
        void init() {
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            window = glfwCreateWindow(100, 100, "title", nullptr, nullptr);
            glfwMakeContextCurrent(window);

            glewExperimental = GL_TRUE;
            glewInit();

            GLuint VAO;
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            GLuint v_shader =
                create_shader(GL_VERTEX_SHADER, vertex_shader_source, "vertex shader");
            GLuint f_shader =
                create_shader(GL_FRAGMENT_SHADER, fragment_shader_source, "fragment shader");

            glEnable(GL_DEPTH_TEST);

            init_buffers();
            glGenBuffers(4, buffers);

            float vertex_positions[vertex_count][4];
            float zeros[vertex_count][4];
            std::memset(zeros, 0, sizeof(zeros));

            for (unsigned int y = 0; y < column_length; y++) {
                for (unsigned int x = 0; x < row_length; x++) {
                    float y_fraction = static_cast<float>(y) / column_length;
                    float radius = (1 - y_fraction) * upper_radius + y_fraction * lower_radius;
                    vertex_positions[y * row_length + x][0] =
                        std::cos(std::numbers::pi_v<float> * 2 * static_cast<float>(x) / row_length)
                        * radius;
                    vertex_positions[y * row_length + x][1] = 0.5f - y_fraction;
                    vertex_positions[y * row_length + x][2] =
                        -std::sin(std::numbers::pi_v<float>
                                  * 2 * static_cast<float>(x) / row_length)
                        * radius;
                }
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[0]);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), vertex_positions,
                         GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[1]);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), vertex_positions,
                         GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[2]);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), nullptr,
                         GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[3]);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), zeros, GL_STATIC_DRAW);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            current_buffer = 0;

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers[0]);

            program = glCreateProgram();
            glAttachShader(program, v_shader);
            glAttachShader(program, f_shader);
            link_program(program, "main program");

            glm::mat4 view_transform(1.0f);
            view_transform = glm::translate(view_transform, glm::vec3(0, 0, -3));
            view_transform = glm::rotate(view_transform, 0.4f, glm::vec3(1, 0, 0));
            view_transform =
                glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f) * view_transform;
            GLuint view_transform_uniform_location =
                glGetUniformLocation(program, "view_transform");
            glProgramUniformMatrix4fv(program, view_transform_uniform_location, 1, GL_FALSE,
                                      glm::value_ptr(view_transform));

            delta_time_uniform_location = glGetUniformLocation(program, "delta_time");
        }

        bool has_finished() {
            return glfwWindowShouldClose(window);
        }

        void display(float delta_time) {
            glUniform1f(delta_time_uniform_location, delta_time);

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glViewport(0, 0, width, height);

            glClearColor(1, 0, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(program);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffers[1 + current_buffer]);
            current_buffer ^= 1;
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffers[1 + current_buffer]);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffers[3]);

            glDrawArrays(GL_TRIANGLES, 0, 6 * row_length * (column_length - 1));

            glFinish();
            glfwSwapBuffers(window);
        }

        void destroy() {
            glfwDestroyWindow(window);
        }
};

int main() {

    Painter pnt;
    pnt.init();

    auto prev_time_point = std::chrono::high_resolution_clock::now();
    while (!pnt.has_finished()) {
        auto cur_time_point = std::chrono::high_resolution_clock::now();
        float elapsed_seconds =
            std::chrono::duration<float>{cur_time_point - prev_time_point}.count();
        prev_time_point = cur_time_point;
        pnt.display(elapsed_seconds);

        glfwPollEvents();
    }

    pnt.destroy();
}
