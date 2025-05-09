#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum { display_type_color, display_type_shadow, num_display_types };

class Painter {
    private:
        GLFWwindow *window = nullptr;
        GLint delta_time_uniform_location = 0;
        GLint light_dir_uniform_location = 0;
        GLuint program = 0;
        GLuint ground_program = 0;
        GLuint cloth_VAO = 0;
        GLuint ground_VAO = 0;
        unsigned int current_buffer = 0;
        GLuint shadow_texture = 0;
        GLuint shadow_color_texture = 0;
        GLuint shadow_framebuffer = 0;

        enum buffer_indices { start_positions, first_positions, second_positions, velocities, num };

        GLuint buffers[buffer_indices::num];

        // uniform buffer that sets the transform to light and disables movement simulation
        GLuint light_display_options_buffer = 0;
        // uniform buffer that sets the transform to view and makes shader simulate movement
        GLuint view_display_options_buffer = 0;

        void init_buffers();

        struct ground_vertex {
            public:
                float position[3];
        };

        void init_ground_VAO();

        void draw_shadows(float delta_time, unsigned int type);
        void draw_to_screen(unsigned int type);

        void init_opengl_window();

        void init_ground_shader_program();
        void init_cloth_shader_program();
        void init_shader_programs();

        void init_view_transform_uniforms();
        void init_light_transform_uniforms();

        void init_shadow_textures_and_framebuffer();

        glm::mat4 construct_view_matrix();

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
