#include "Painter.hpp"

namespace {
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
} // namespace

void Painter::init_buffers() {

    float vertex_positions[vertex_count][4];

    glGenBuffers(4, buffers);
    std::memset(zeros, 0, sizeof(zeros));

    // init vertex data
    for (unsigned int y = 0; y < column_length; y++) {
        for (unsigned int x = 0; x < row_length; x++) {
            float y_fraction = static_cast<float>(y) / column_length;
            float radius = (1 - y_fraction) * upper_radius + y_fraction * lower_radius;
            vertex_positions[y * row_length + x][0] =
                std::cos(std::numbers::pi_v<float> * 2 * static_cast<float>(x) / row_length)
                * radius;
            vertex_positions[y * row_length + x][1] = 0.5f - y_fraction;
            vertex_positions[y * row_length + x][2] =
                -std::sin(std::numbers::pi_v<float> * 2 * static_cast<float>(x) / row_length)
                * radius;
            vertex_positions[y * row_length + x][3] = 1;
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), vertex_positions,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), vertex_positions,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[2]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertex_positions), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[3]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(zeros), zeros, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    current_buffer = 0;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers[0]);
}

void Painter::init_ground_VAO() {
    glGenVertexArrays(1, &ground_VAO);
    glBindVertexArray(ground_VAO);

    ground_vertex vertices[] = {
        {-2, -1,  2},
        { 2, -1,  2},
        {-2, -1, -2},
        { 2, -1, -2}
    };

    GLuint ground_vertices_buffer;
    glGenBuffers(1, &ground_vertices_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, ground_vertices_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ground_vertex),
                          reinterpret_cast<void *>(offsetof(ground_vertex, position)));
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void Painter::init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(100, 100, "title", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    glewInit();

    glGenVertexArrays(1, &cloth_VAO);

    GLuint v_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source, "vertex shader");
    GLuint f_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source, "fragment shader");

    glEnable(GL_DEPTH_TEST);

    init_buffers();
    init_ground_VAO();

    program = glCreateProgram();
    glAttachShader(program, v_shader);
    glAttachShader(program, f_shader);
    link_program(program, "main program");

    GLuint v_shader_ground =
        create_shader(GL_VERTEX_SHADER, vertex_ground_shader_source, "ground vertex shader");
    GLuint f_shader_ground =
        create_shader(GL_FRAGMENT_SHADER, fragment_ground_shader_source, "ground fragment shader");
    ground_program = glCreateProgram();
    glAttachShader(ground_program, v_shader_ground);
    glAttachShader(ground_program, f_shader_ground);
    link_program(ground_program, "ground program");

    glm::mat4 view_transform = glm::mat4(1.0f);
    view_transform = glm::translate(view_transform, glm::vec3(0, 0, -3));
    view_transform = glm::rotate(view_transform, 0.4f, glm::vec3(1, 0, 0));
    view_transform = glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f) * view_transform;
    GLuint view_transform_uniform_location = glGetUniformLocation(program, "view_transform");
    glProgramUniformMatrix4fv(program, view_transform_uniform_location, 1, GL_FALSE,
                              glm::value_ptr(view_transform));

    GLuint view_transform_ground_uniform_location =
        glGetUniformLocation(ground_program, "view_transform");
    glProgramUniformMatrix4fv(ground_program, view_transform_ground_uniform_location, 1, GL_FALSE,
                              glm::value_ptr(view_transform));

    delta_time_uniform_location = glGetUniformLocation(program, "delta_time");

    glm::mat4 light_transform =
        glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 5.0f)
        * glm::lookAt(light_direction, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

    GLuint light_ground_transform_uniform_location =
        glGetUniformLocation(ground_program, "light_transform");
    glProgramUniformMatrix4fv(ground_program, light_ground_transform_uniform_location, 1, GL_FALSE,
                              glm::value_ptr(light_transform));

    glGenBuffers(1, &view_display_options_buffer);
    glGenBuffers(1, &light_display_options_buffer);

    glBindBuffer(GL_UNIFORM_BUFFER, light_display_options_buffer);
    glBufferData(GL_UNIFORM_BUFFER, 4 * 4 * sizeof(GLfloat) + sizeof(GLuint), nullptr,
                 GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * 4 * sizeof(GLfloat), glm::value_ptr(light_transform));
    GLuint dont_update = false;
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * 4 * sizeof(GLfloat), sizeof(GLuint), &dont_update);

    glBindBuffer(GL_UNIFORM_BUFFER, view_display_options_buffer);
    glBufferData(GL_UNIFORM_BUFFER, 4 * 4 * sizeof(GLfloat) + sizeof(GLuint), nullptr,
                 GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * 4 * sizeof(GLfloat), glm::value_ptr(view_transform));
    GLuint do_update = true;
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * 4 * sizeof(GLfloat), sizeof(GLuint), &do_update);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenTextures(1, &shadow_texture);
    glGenTextures(1, &shadow_color_texture);
    glGenFramebuffers(1, &shadow_framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer);

    glBindTexture(GL_TEXTURE_2D, shadow_texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, shadow_map_size, shadow_map_size);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_texture, 0);

    glBindTexture(GL_TEXTURE_2D, shadow_color_texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, shadow_map_size, shadow_map_size);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadow_color_texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void Painter::display(float delta_time, unsigned int type) {
    glProgramUniform1f(program, delta_time_uniform_location, delta_time);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_framebuffer);
    glViewport(0, 0, shadow_map_size, shadow_map_size);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    if (type == display_type_color) {
        glDisable(GL_DEPTH_TEST);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glUseProgram(program);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, light_display_options_buffer);

    glBindVertexArray(cloth_VAO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffers[1 + current_buffer]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * row_length * (column_length - 1));

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, view_display_options_buffer);

    glBindVertexArray(cloth_VAO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffers[1 + current_buffer]);
    current_buffer ^= 1;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buffers[1 + current_buffer]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buffers[3]);

    glDrawArrays(GL_TRIANGLES, 0, 6 * row_length * (column_length - 1));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadow_color_texture);
    glUseProgram(ground_program);
    glUniform1ui(glGetUniformLocation(ground_program, "type"), type);
    glBindVertexArray(ground_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(window);
}
