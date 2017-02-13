#include <iostream>
#include <cstdlib>
#include <vector>
#include <utility>

#include <glm/gtx/rotate_vector.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <SOIL.h>

#include "Shader.h"
#include "DataStructures.h"
#include "Bezier.h"

using namespace std;
using namespace glm;

GLuint WIDTH = 800;
GLuint HEIGHT = 600;

int MOUSEX;
int MOUSEY;

Shader defaultShader;
Shader textureShader;

GLuint surfaceVAO;
GLuint surfaceVBO;

GLuint baseVAO;
GLuint baseVBO;

GLuint curveVAO;
GLuint curveVBO;

GLuint axisVAO;
GLuint axisVBO;

GLuint texture;

vector<ColorVertex> baseLineVertices;
vector<ColorVertex> bezierCurveVertices;
vector<TextureVertex> surfaceVertices;

bool isRotated = false;
char surfaceMode = 't';

//----------------- ROTATION ---------------------

void calculateSurfeceVertices(const char axis) {
	vector<vector<TextureVertex>> surfVec;

	for (float t = 0; t <= 350; t += 10) {
		vector<TextureVertex> curveVec;
		for (vector<ColorVertex>::iterator it = bezierCurveVertices.begin();
				it != bezierCurveVertices.end(); ++it) {
			vec3 rvec;

			if (axis == 'y') {
				rvec = rotateY(it->position, (GLfloat) t);
			} else {
				rvec = rotateX(it->position, (GLfloat) t);
			}

			curveVec.push_back(TextureVertex(rvec, vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f)));
		}
		surfVec.push_back(curveVec);
	}

	surfaceVertices.clear();

	for (int i = 0; i < surfVec.size() - 1; i++) {
		for (int j = 0; j < surfVec[i].size() - 1; j++) {
			surfaceVertices.push_back(TextureVertex(surfVec[i][j+1].position, surfVec[i][j+1].color, vec2(0.0f, 0.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i+1][j+1].position, surfVec[i+1][j+1].color, vec2(1.0f, 0.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i+1][j].position, surfVec[i+1][j].color, vec2(1.0f, 1.0f)));

			surfaceVertices.push_back(TextureVertex(surfVec[i+1][j].position, surfVec[i+1][j].color, vec2(1.0f, 1.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i][j].position, surfVec[i][j].color, vec2(0.0f, 1.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i][j+1].position, surfVec[i][j+1].color, vec2(0.0f, 0.0f)));
		}
	}

	const int p = surfVec.size() - 1;
	for (int j = 0; j < surfVec.back().size() - 1; j++) {
		surfaceVertices.push_back(TextureVertex(surfVec[p][j+1].position, surfVec[p][j+1].color, vec2(0.0f, 0.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[0][j+1].position, surfVec[0][j+1].color, vec2(1.0f, 0.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[0][j].position, surfVec[0][j].color, vec2(1.0f, 1.0f)));

		surfaceVertices.push_back(TextureVertex(surfVec[0][j].position, surfVec[0][j].color, vec2(1.0f, 1.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[p][j].position, surfVec[p][j].color, vec2(0.0f, 1.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[p][j+1].position, surfVec[p][j+1].color, vec2(0.0f, 0.0f)));
	}
}

//----------------- ROTATION ---------------------

//----------------- MISELANIOUS ---------------------

void initGL(void) {
	glClearColor(0.0, 0.0, 0.0, 0.0);

	if (glewInit()) {
		cout << "Unable to initialize GLEW ... exiting" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	cout << "Opengl Version: " << glGetString(GL_VERSION) << endl;
	cout << "Opengl Vendor : " << glGetString(GL_VENDOR) << endl;
	cout << "Opengl Render : " << glGetString(GL_RENDERER) << endl;
	cout << "Opengl Shading Language Version : "
			<< glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void calculateBezier() {
	bezierCurveVertices.clear();

	if (baseLineVertices.size() >= 2) {
		vec3 color = vec3(0.8, 0.6, 0.1);

		ColorVertex p1 = baseLineVertices[0];
		p1.color = color;

		bezierCurveVertices.push_back(p1);

		for (float t = 0.0; t <= 1.0; t += 0.02) {
			ColorVertex p2 = evaluateBezier(t, &baseLineVertices);
			p2.color = color;
			bezierCurveVertices.push_back(p2);
			p1 = p2;
		}
	}
}

//----------------- MISELANIOUS ---------------------

//----------------- CREATE AND UPDATE ELEMENTS--------

void changeTexture(const char* tex) {
	glGenTextures(1, &texture);

	int width, height;
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unsigned char *image = SOIL_load_image(tex, &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
}

void createSurfaceWithTex() {
	glGenVertexArrays(1, &surfaceVAO);
	glGenBuffers(1, &surfaceVBO);

	glBindVertexArray(surfaceVAO);

	glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TextureVertex) * surfaceVertices.size(),
			&surfaceVertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextureVertex),
			(void*) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TextureVertex),
			(void*) 12);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TextureVertex),
			(void*) 24);

	glBindVertexArray(0);
}

void createAxis() {
	glGenVertexArrays(1, &axisVAO);
	glBindVertexArray(axisVAO);

	glGenBuffers(1, &axisVBO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex),
			(void*) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex),
			(void*) 12);

	ColorVertex vertices[] = {
			ColorVertex(vec3(-1.0f, 0, 0), vec3(1.0f, 0, 0)),
			ColorVertex(vec3(1.0f, 0, 0), vec3(1.0f, 0, 0)),
			ColorVertex(vec3(0, -1.0, 0), vec3(0, 1.0f, 0)),
			ColorVertex(vec3(0, 1.0, 0), vec3(0, 1.0f, 0)),
			ColorVertex(vec3(0, 0, -1.0), vec3(0, 0, 1.0f)),
			ColorVertex(vec3(0, 0, 1.0), vec3(0, 0, 1.0f))
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertex) * 6, &vertices[0],
			GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void createBase() {
	glGenVertexArrays(1, &baseVAO);
	glBindVertexArray(baseVAO);

	glGenBuffers(1, &baseVBO);
	glBindBuffer(GL_ARRAY_BUFFER, baseVBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex),
			(void*) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex),
			(void*) 12);

	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void createCurve() {
	glGenVertexArrays(1, &curveVAO);
	glBindVertexArray(curveVAO);

	glGenBuffers(1, &curveVBO);
	glBindBuffer(GL_ARRAY_BUFFER, curveVBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex),
			(void*) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex),
			(void*) 12);

	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void updatePoints() {
	glBindVertexArray(baseVAO);
	glBindBuffer(GL_ARRAY_BUFFER, baseVBO);

	glBufferData(GL_ARRAY_BUFFER,
			sizeof(ColorVertex) * baseLineVertices.size(),
			&baseLineVertices[0], GL_STREAM_DRAW);

	glBindVertexArray(curveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, curveVBO);

	glBufferData(GL_ARRAY_BUFFER,
			sizeof(ColorVertex) * bezierCurveVertices.size(),
			&bezierCurveVertices[0], GL_STREAM_DRAW);

	glBindVertexArray(0);
}

//----------------- CREATE AND UPDATE ELEMENTS--------

//----------------- DRAW COMMANDS ---------------------

void drawSurfaceWithTex() {
	textureShader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(glGetUniformLocation(textureShader.program, "ourTexture" ), 0);

	glBindVertexArray(surfaceVAO);
	glDrawArrays(GL_TRIANGLES, 0, surfaceVertices.size());
	glBindVertexArray(0);
}

void drawSurfaceWithVertices() {
	defaultShader.use();
	glBindVertexArray(surfaceVAO);
	glPointSize(2);
	glDrawArrays(GL_POINTS, 0, surfaceVertices.size());
	glBindVertexArray(0);
}

void drawSurfaceWithEdges() {
	defaultShader.use();
	glBindVertexArray(surfaceVAO);
	glDrawArrays(GL_LINE_STRIP, 0, surfaceVertices.size());
	glBindVertexArray(0);
}

void drawAxis() {
	defaultShader.use();
	glBindVertexArray(axisVAO);
	glDrawArrays(GL_LINES, 0, 6);
	glBindVertexArray(0);
}

void drawBaseLines() {
	defaultShader.use();
	glBindVertexArray(baseVAO);
	glPointSize(6.0);
	glDrawArrays(GL_POINTS, 0, baseLineVertices.size());
	glDrawArrays(GL_LINE_STRIP, 0, baseLineVertices.size());
	glBindVertexArray(0);
}

void drawCurve() {
	defaultShader.use();
	glBindVertexArray(curveVAO);
	glDrawArrays(GL_LINE_STRIP, 0, bezierCurveVertices.size());
	glBindVertexArray(0);
}

//----------------- DRAW COMMANDS ---------------------

//----------------- EVENT HANDLERS ---------------------

void mousePosFunc(GLFWwindow* window, double xpos, double ypos) {
	MOUSEX = xpos;
	MOUSEY = ypos;
}

void mouseClickFunc(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		float x = (MOUSEX * 1.0f / WIDTH * 2 - 1);
		float y = (-MOUSEY * 1.0f / HEIGHT * 2 + 1);

		ColorVertex vf(vec3(x, y, 0.0), vec3(1, 1, 1));

		baseLineVertices.push_back(vf);
		calculateBezier();
		updatePoints();
	}
}

void keyPressFunc(GLFWwindow* window, int key, int scancode, int action,
		int mods) {
	if (action != GLFW_PRESS) {
		return;
	}

	switch (key) {

	case GLFW_KEY_Q:
		baseLineVertices.pop_back();
		calculateBezier();
		updatePoints();
		break;

	case GLFW_KEY_X:
		if (bezierCurveVertices.size() > 0) {
			isRotated = true;
			calculateSurfeceVertices('x');
			createSurfaceWithTex();
		}
		break;

	case GLFW_KEY_Y:
		if (bezierCurveVertices.size() > 0) {
			isRotated = true;
			calculateSurfeceVertices('y');
			createSurfaceWithTex();
		}
		break;

	case GLFW_KEY_E:
		baseLineVertices.clear();
		bezierCurveVertices.clear();
		surfaceVertices.clear();

		updatePoints();
		isRotated = false;
		break;

	case GLFW_KEY_1:
		surfaceMode = 't';
		glDeleteTextures(1, &texture);
		changeTexture("res/textures/red.jpg");
		break;

	case GLFW_KEY_2:
		surfaceMode = 't';
		glDeleteTextures(1, &texture);
		changeTexture("res/textures/metal.jpg");
		break;

	case GLFW_KEY_3:
		surfaceMode = 't';
		glDeleteTextures(1, &texture);
		changeTexture("res/textures/wood.jpg");
		break;

	case GLFW_KEY_4:
		glDeleteTextures(1, &texture);
		surfaceMode = 'v';
		break;

	case GLFW_KEY_5:
		glDeleteTextures(1, &texture);
		surfaceMode = 'e';
		break;

	default:
		break;
	}
}

//----------------- EVENT HANDLERS ---------------------

void display() {
	drawAxis();

	if (isRotated) {

		switch (surfaceMode) {

		case 't':
			drawSurfaceWithTex();
			break;

		case 'v':
			drawSurfaceWithVertices();
			break;

		case 'e' :
			drawSurfaceWithEdges();
			break;
		}

	} else {
		drawBaseLines();
		drawCurve();
	}
}

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Computação Visual",
			nullptr, nullptr);

	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (nullptr == window) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit()) {
		cout << "Failed to initialize GLEW" << endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, screenWidth, screenHeight);

	glfwSetCursorPosCallback(window, mousePosFunc);
	glfwSetMouseButtonCallback(window, mouseClickFunc);
	glfwSetKeyCallback(window, keyPressFunc);

	initGL();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	defaultShader.load("res/shaders/default.vs", "res/shaders/default.fs");
	textureShader.load("res/shaders/texture.vs", "res/shaders/texture.fs");

	createAxis();
	createBase();
	createCurve();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glLoadIdentity();

		glClear(GL_COLOR_BUFFER_BIT);

		display();

		glfwSwapBuffers(window);
	}

	glDeleteBuffers(1, &axisVBO);
	glDeleteBuffers(1, &baseVBO);
	glDeleteBuffers(1, &curveVBO);
	glDeleteBuffers(1, &surfaceVBO);

	glDeleteVertexArrays(1, &axisVAO);
	glDeleteVertexArrays(1, &baseVAO);
	glDeleteVertexArrays(1, &curveVAO);
	glDeleteVertexArrays(1, &surfaceVAO);

	glDeleteTextures(1, &texture);

	glfwTerminate();

	return EXIT_SUCCESS;
}
