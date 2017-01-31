#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Point {
public:
    float x, y;

    void setxy(float x2, float y2) {
        x = x2;
        y = y2;
    }

    const Point & operator=(const Point &point) {
        x = point.x;
        y = point.y;
        return *this;
    }
};

int mouse_position_x;
int mouse_position_y;

int screen_width;
int screen_height;

std::vector<Point> points_array;

Point ccoord_to_vcoord(float clientX, float clientY) {
    Point p;
    p.x = (clientX * 1.0f / screen_width * 2 - 1);
    p.y = (-clientY * 1.0f / screen_height * 2 + 1);

    return p;
}

void bezierCoefficients(int n,int *c)
{
    int k,i;
    for (k=0; k<=n; k++) {
        c[k]=1;

        for (i=n;i>=k+1;i--) {
        c[k]*=i;
        }

        for (i=n-k;i>=2;i--) {
            c[k]/=i;
        }
    }
}

void draw_bezier_curve() {
    int cp[4][2]={{10,10},{100,200},{200,50},{300,300}};
    int c[4],k,n=3;
    float x,y,u,blend;

    bezierCoefficients(n,c);

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0,0.0,0.0);
    glLineWidth(5.0);
    glBegin(GL_LINE_STRIP);

    for (u=0;u<1.0;u+=0.01) {
        x=0;y=0;
        for (k=0;k<4;k++) {
            blend = c[k] * pow(u,k) * pow(1-u, n-k);
            x += cp[k][0] * blend;
            y += cp[k][1] * blend;
        }

        Point p = ccoord_to_vcoord(x, y);
        glVertex2f(p.x, p.y);
    }

    glEnd();
    glFlush();
}

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
    } else if (key == GLFW_KEY_C) {
        draw_bezier_curve();
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    Point p = ccoord_to_vcoord(mouse_position_x, mouse_position_y);
    printf("%f %f\n", p.x, p.y);
    points_array.push_back(p);
}

void draw_points() {
    glEnable(GL_POINT_SMOOTH);
    glPointSize(3.0f);

    glBegin(GL_POINTS);
    for (std::vector<Point>::iterator it = points_array.begin(); it != points_array.end(); ++it) {
        glVertex2f(it->x, it->y);
    }
    glEnd();

    glDisable(GL_POINT_SMOOTH);
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

    glfwGetWindowSize(window, &screen_width, &screen_height);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCharCallback(window, character_callback);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        draw_points();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}