#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <array>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

int mouse_position_x;
int mouse_position_y;

std::vector<std::array<float, 2>> points_array;

void error_callback(int error, const char* description) {
    fprintf(stderr, description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    } else if (key == GLFW_KEY_E) {
        points_array.clear();
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }
    
    int width, height;
        
    glfwGetWindowSize(window, &width, &height);

    float virtualX = (mouse_position_x * 1.0f / width * 2 - 1);
    float virtualY = (-mouse_position_y * 1.0f / height * 2 + 1);

    points_array.push_back({virtualX, virtualY});
}

void draw_points() {
    glEnable(GL_POINT_SMOOTH);
    glPointSize(3.0f);

    glBegin(GL_POINTS);
    for (std::vector<std::array<float, 2>>::iterator it = points_array.begin(); it != points_array.end(); ++it) {
        glVertex2f((*it)[0], (*it)[1]);
    }
    glEnd();

    glDisable(GL_POINT_SMOOTH);
}

void draw_lines() {
    int threshold = points_array.size() - 1;
    int index = 0;

    glBegin(GL_LINES);
    for (std::vector<std::array<float, 2>>::iterator it = points_array.begin(); it != points_array.end(); ++it) {
        if (index > 0 && index < threshold) {
           glVertex2f((*it)[0], (*it)[1]); 
        }
        glVertex2f((*it)[0], (*it)[1]);
        index++;
    }
    glEnd();
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    mouse_position_x = xpos;
    mouse_position_y = ypos;
}  

int main() {
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // window = glfwCreateWindow(640, 480, "Computação Visual - Atividade 1", glfwGetPrimaryMonitor(), NULL);
    window = glfwCreateWindow(960, 680, "Computação Visual - Atividade 1", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCharCallback(window, character_callback);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        draw_points();
        draw_lines();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}