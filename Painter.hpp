#include "constants.hpp"
#include "fragment_shader.hpp"
#include "fragment_shader_ground.hpp"
#include "vertex_shader.hpp"
#include "vertex_shader_ground.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <numbers>
#include <sstream>
#include <stdexcept>

enum { display_type_color, display_type_shadow, num_display_types };

class Painter {
    private:
        GLFWwindow *window = nullptr;
        GLint delta_time_uniform_location = 0;
        GLuint program = 0;
        GLuint ground_program = 0;
        GLuint cloth_VAO = 0;
        GLuint ground_VAO = 0;
        unsigned int current_buffer = 0;
        GLuint shadow_texture = 0;
        GLuint shadow_color_texture = 0;
        GLuint shadow_framebuffer = 0;
        GLuint buffers[4];

        GLuint light_display_options_buffer = 0;
        GLuint view_display_options_buffer = 0;

        float vertex_positions[vertex_count][4]; // i dont know why these cant be
                                                 // local variables, but then it doesn't work
        float zeros[vertex_count][4];

        void init_buffers();

        struct ground_vertex {
            public:
                float position[3];
        };

        void init_ground_VAO();

    public:

        GLFWwindow *get_window() {
            return window;
        }

        void init();

        bool has_finished() {
            return glfwWindowShouldClose(window);
        }

        void display(float delta_time, unsigned int type);

        void destroy() {
            glfwDestroyWindow(window);
        }
};
