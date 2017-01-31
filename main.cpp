#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

int mouse_position_x;
int mouse_position_y;

static void error_callback(int error, const char* description) {
    fprintf(stderr, description);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }
    
    int width, height;
        
    glfwGetWindowSize(window, &width, &height);

    float virtualX = (mouse_position_x * 1.0f / width * 2 - 1);
    float virtualY = (-mouse_position_y * 1.0f / height * 2 + 1);

}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouse_position_x = xpos;
    mouse_position_y = ypos;
}

static void character_callback(GLFWwindow* window, unsigned int codepoint) {
}

int main() {
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // window = glfwCreateWindow(640, 480, "Computação Visual - Atividade 1", glfwGetPrimaryMonitor(), NULL);
    window = glfwCreateWindow(640, 480, "Computação Visual - Atividade 1", NULL, NULL);

    if (window == NULL) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glewExperimental = true;

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCharCallback(window, character_callback);

    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}