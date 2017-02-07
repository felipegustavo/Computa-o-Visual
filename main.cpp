#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <GL/glew.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

#define PI 2 * M_PI

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

std::vector<Point> points;
std::vector<Point> curve;

int winWidth = 800;
int winHeight = 600;

bool isRotated = false;

int factorial(int n) {
    if (n<=1) {
        return(1);
    } else {
        n=n * factorial(n-1);
    }
    return n;
}

float binomialCoff(float n,float k) {
    float ans;
    ans = factorial(n) / (factorial(k)*factorial(n-k));
    return ans;
}

Point getNextBezierPoint(float t) {
    Point p;
    p.x = 0; p.y = 0;   

    int size = points.size();

    for (int i = 0; i < size; i++) {
        p.x = p.x + binomialCoff((float)(size - 1), (float)i) * pow(t, (float)i) * pow((1 - t), (size - 1 - i)) * points[i].x;
        p.y = p.y + binomialCoff((float)(size - 1), (float)i) * pow(t, (float)i) * pow((1 - t), (size - 1 - i)) * points[i].y;
    }

    return p;
}

Point ccoordToVcoord(float clientX, float clientY) {
    Point p;
    p.x = (clientX * 1.0f / winWidth * 2 - 1);
    p.y = (-clientY * 1.0f / winHeight * 2 + 1);
    p.z = 0;

    return p;
}

void drawDot(Point p) {
    glPointSize(8.0);
    glBegin(GL_POINTS);
    glVertex3f(p.x, p.y, p.z);
    glEnd();
}

void drawLine(Point p1, Point p2) {
    glLineWidth(5.0);
    glBegin(GL_LINES);
    glVertex3f(p1.x, p1.y, p1.z);
    glVertex3f(p2.x, p2.y, p2.z);
    glEnd();
}

void calculateCurve() {
	if (isRotated) {
		return;
	}

	if (points.size() > 2) {
        curve.clear();

        Point p1 = points[0];
        curve.push_back(p1);

        for(float t = 0.0;t <= 1.0; t += 0.02) {
            Point p2 = getNextBezierPoint(t);
            curve.push_back(p2);
            p1 = p2;
        }
    } else {
    	curve.clear();
    }
}

void calculateSurface(unsigned char axis) {
	if (curve.empty()) {
		return;
	}

	points.clear();

	for (vector<Point>::iterator it = curve.begin(); it != curve.end(); ++it) {
		for (float t = 0; t <= PI; t += 0.5) {
			Point p;

			if (axis == 'x') {
				p.x = it->x;
				p.y = it->y * cos(t) - it->z * sin(t);
				p.z = it->y * sin(t) + it->z * cos(t);
			} else if (axis == 'y') {
				p.x = it->z * sin(t) + it->x * cos(t);
				p.y = it->y;
				p.z = it->z * cos(t) - it->x * sin(t);
			} else {
				p.x = it->x * cos(t) - it->y * sin(t);
				p.y = it->x * sin(t) + it->y * cos(t);
				p.z = it->z;
			}

			points.push_back(p);
		}
	}
}

void drawCurve(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	if (points.size() > 0) {
		drawDot(points[0]);
	}

    for (int i = 1; i < points.size(); i++) {
        drawDot(points[i]);
        drawLine(points[i-1], points[i]);
    }

    glColor3f(0.2,1.0,0.0);
    for (int i = 1; i < curve.size(); i++) {
    	drawLine(curve[i-1], curve[i]);
    }
    glColor3f(1.0, 1.0, 1.0);

	glutSwapBuffers();  
}

void drawSurface(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	for (vector<Point>::iterator it = points.begin(); it != points.end(); ++it) {
		drawDot((*it));
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
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !isRotated) {
		Point p = ccoordToVcoord(x, y);
   		points.push_back(p);
   		calculateCurve();
	}
}

void processNormalKeys(unsigned char key, int x, int y) {
	switch (key) {
		case 27 :
			exit(EXIT_SUCCESS);
			break;

		case 'q' :
			points.clear();
			curve.clear();
			isRotated = false;
			break;

		case 'w' :
			if (points.size() > 0 && !isRotated) {
				points.pop_back();
			}
			calculateCurve();
			break;

		case 'x' :
		case 'y' :
		case 'z' :
			calculateSurface(key);
			isRotated = true;
			break;
	}
}

void renderScene(void) {
	if (isRotated) {
		drawSurface();
	} else {
		drawCurve();
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
	glutKeyboardFunc(processNormalKeys);

	initGL();

	glutMainLoop();

	return EXIT_SUCCESS;
}