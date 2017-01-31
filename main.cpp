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
std::vector<Point> curve_array;

bool draw_revolution = false;

int factorial(int n) {
    if (n<=1) {
        return(1);
    } else {
        n=n * factorial(n-1);
    }

    return n;
}

float binomial_coff(float n,float k) {
    float ans;
    ans = factorial(n) / (factorial(k)*factorial(n-k));
    return ans;
}

Point get_next_bezier_point(float t) {
    Point p;
    p.x = 0; p.y = 0;   

    int size = points_array.size();

    for (int i = 0; i < size; i++) {
        p.x = p.x + binomial_coff((float)(size - 1), (float)i) * pow(t, (float)i) * pow((1 - t), (size - 1 - i)) * points_array[i].x;
        p.y = p.y + binomial_coff((float)(size - 1), (float)i) * pow(t, (float)i) * pow((1 - t), (size - 1 - i)) * points_array[i].y;
    }

    return p;
}

Point ccoord_to_vcoord(float clientX, float clientY) {
    Point p;
    p.x = (clientX * 1.0f / screen_width * 2 - 1);
    p.y = (-clientY * 1.0f / screen_height * 2 + 1);

    return p;
}

void draw_dot(float x, float y) {
    glPointSize(8.0);
    glBegin(GL_POINTS);
    glVertex3f(x, y, 0);
    glEnd();
}

void draw_line(Point p1, Point p2) {
    glLineWidth(5.0);
    glBegin(GL_LINES);
    glVertex3f(p1.x, p1.y, 0);
    glVertex3f(p2.x, p2.y, 0);
    glEnd();
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
        curve_array.clear();
        draw_revolution = false;
    } else if(key == GLFW_KEY_R) {
        draw_revolution = true;
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    Point p = ccoord_to_vcoord(mouse_position_x, mouse_position_y);
    points_array.push_back(p);
}

void draw_curve() {
    glColor3f(1.0, 1.0, 1.0);
    for (int i = 0; i < points_array.size(); i++) {
        draw_dot(points_array[i].x, points_array[i].y);
        if (i > 0) {
            draw_line(points_array[i-1], points_array[i]);
        }
    }

    if (points_array.size() > 2) {
        glColor3f(0.2,1.0,0.0);
        curve_array.clear();

        Point p1 = points_array[0];
        curve_array.push_back(p1);

        for(float t = 0.0;t <= 1.0; t += 0.02) {
            Point p2 = get_next_bezier_point(t);
            curve_array.push_back(p2);

            draw_line(p1, p2);
            p1 = p2;
        }

        glColor3f(1.0, 1.0, 1.0);
    }
}

void draw_surface() {
    glPushMatrix();
    glRotatef(90, 0., 1., 0.);
    glColor3f(0.2,1.0,0.0);
    for (int i = 1; i < curve_array.size(); i++) {
        draw_line(curve_array[i-1], curve_array[i]);
    }
    glPopMatrix();
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
        glClear(GL_COLOR_BUFFER_BIT);

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

        if (!draw_revolution) {
            draw_curve();
        } else {
            draw_surface();
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}