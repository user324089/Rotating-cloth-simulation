#include "Painter.hpp"

#include <chrono>

unsigned int current_display_type = display_type_color;

void key_callback(GLFWwindow *, int key, int, int action, int) {
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        current_display_type++;
        current_display_type %= num_display_types;
    }
}

int main() {

    Painter pnt;
    pnt.init();

    glfwSetKeyCallback(pnt.get_window(), key_callback);

    auto prev_time_point = std::chrono::high_resolution_clock::now();
    while (!pnt.has_finished()) {
        auto cur_time_point = std::chrono::high_resolution_clock::now();
        float elapsed_seconds =
            std::chrono::duration<float>{cur_time_point - prev_time_point}.count();
        prev_time_point = cur_time_point;
        pnt.display(elapsed_seconds, current_display_type);

        glfwPollEvents();
    }

    pnt.destroy();
}
