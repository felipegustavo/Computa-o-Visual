#include <iostream>
#include <vector>
#include <cmath>
#include <GL/glew.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

class Point {
public:
    float x, y, z;

    void setxyz(float x2, float y2, float z2) {
        x = x2;
        y = y2;
        z = z2;
    }

    const Point & operator=(const Point &point) {
        x = point.x;
        y = point.y;
        z = point.z;
        return *this;
    }
};

std::vector<Point> points_array;
std::vector<Point> curve_array;

int winWidth = 640;
int winHeight = 480;

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
    p.x = (clientX * 1.0f / winWidth * 2 - 1);
    p.y = (-clientY * 1.0f / winHeight * 2 + 1);
    p.z = 0;

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
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p2.x, p2.y, p2.z);
    glEnd();
}

void renderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

	glutSwapBuffers();
}

void initGL(void) {
	glClearColor(0.0, 0.0, 0.0, 0.0);

	if (glewInit()) {
		cout << "Can't initialize GLEW." << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Status" << endl;
	cout << "GLEW version: " << glewGetString(GLEW_VERSION) << endl;
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "OpenGL render: " << glGetString(GL_RENDERER) << endl;
	cout << "OpenGL shading language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void reshape(int w, int h) {
	winWidth = w;
	winHeight = h;

	glViewport(0, 0, winWidth, winHeight);
	glutPostRedisplay();
}

void mouseClick(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		Point p = ccoord_to_vcoord(x, y);
   		points_array.push_back(p);
	}
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("Computacao Visual - Atividade 1");

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseClick);

	initGL();

	glutMainLoop();

	return 1;
}