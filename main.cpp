#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <utility>
#include <cmath>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <SOIL.h>

#include "Shader.h"
#include "DataStructures.h"
#include "Bezier.h"
#include "Camera.h"

using namespace std;
using namespace glm;

bool firstMouse = true;

GLuint WIDTH = 1280;
GLuint HEIGHT = 720;

int SWIDTH, SHEIGHT;

Camera camera(vec3(0.0f, 0.0f, 1.1f));

GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;

bool keys[1024];

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int MOUSEX;
int MOUSEY;

Shader defaultShader;
Shader textureShader;
Shader surfaceShader;
Shader ctrlShader;

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

	int mm = 0;
	for (float t = 0; t <= 6.3; t += 0.2) {
		mm++;
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
			surfaceVertices.push_back(TextureVertex(surfVec[i][j + 1].position, surfVec[i][j + 1].color, vec2(0.0f, 0.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i + 1][j + 1].position, surfVec[i + 1][j + 1].color, vec2(1.0f, 0.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i + 1][j].position,	surfVec[i + 1][j].color, vec2(1.0f, 1.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i + 1][j].position, surfVec[i + 1][j].color, vec2(1.0f, 1.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i][j].position, surfVec[i][j].color, vec2(0.0f, 1.0f)));
			surfaceVertices.push_back(TextureVertex(surfVec[i][j + 1].position, surfVec[i][j + 1].color, vec2(0.0f, 0.0f)));
		}
	}

	const int p = surfVec.size() - 1;
	for (int j = 0; j < surfVec.back().size() - 1; j++) {
		surfaceVertices.push_back(TextureVertex(surfVec[p][j + 1].position, surfVec[p][j + 1].color, vec2(0.0f, 0.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[0][j + 1].position, surfVec[0][j + 1].color, vec2(1.0f, 0.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[0][j].position, surfVec[0][j].color, vec2(1.0f, 1.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[0][j].position, surfVec[0][j].color, vec2(1.0f, 1.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[p][j].position, surfVec[p][j].color, vec2(0.0f, 1.0f)));
		surfaceVertices.push_back(TextureVertex(surfVec[p][j + 1].position,	surfVec[p][j + 1].color, vec2(0.0f, 0.0f)));
	}
}

//----------------- ROTATION ---------------------

//----------------- MISELANIOUS ---------------------

void doMovement() {
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) {
		camera.processKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) {
		camera.processKeyboard(BACKWARD, deltaTime);
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {
		camera.processKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
		camera.processKeyboard(RIGHT, deltaTime);
	}
}

void prepareCamera(GLuint prog) {
	mat4 projection = perspective(camera.zoom, (GLfloat) SWIDTH / (GLfloat) SHEIGHT, 0.1f, 1000.0f);

	mat4 view = camera.getViewMatrix();

	GLint viewLoc = glGetUniformLocation(prog, "view");
	GLint projLoc = glGetUniformLocation(prog, "projection");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

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
	cout << "Opengl Shading Language Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned char *image = SOIL_load_image(tex, &width, &height, 0,
			SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
}

void createSurfaceWithTex() {
	glGenVertexArrays(1, &surfaceVAO);
	glGenBuffers(1, &surfaceVBO);

	glBindVertexArray(surfaceVAO);

	glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);
	glBufferData(GL_ARRAY_BUFFER,
			sizeof(TextureVertex) * surfaceVertices.size(), &surfaceVertices[0],
			GL_STATIC_DRAW);

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
			ColorVertex(vec3(-100.0f, 0, 0), vec3(1.0f, 0, 0)),
			ColorVertex(vec3(1000.0f, 0, 0), vec3(1.0f, 0, 0)),
			ColorVertex(vec3(0, -1000.0, 0), vec3(0, 1.0f, 0)),
			ColorVertex(vec3(0, 1000.0, 0), vec3(0, 1.0f, 0)),
			ColorVertex(vec3(0, 0, -1000.0), vec3(0, 0, 1.0f)),
			ColorVertex(vec3(0, 0, 1000.0), vec3(0, 0, 1.0f))
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

	glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertex) * baseLineVertices.size(),
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
	glUniform1i(glGetUniformLocation(textureShader.program, "ourTexture"), 0);

	prepareCamera(textureShader.program);

	glBindVertexArray(surfaceVAO);

	glDrawArrays(GL_TRIANGLES, 0, surfaceVertices.size());

	glBindVertexArray(0);
	glUseProgram(0);
}

void drawSurfaceWithVertices() {
	surfaceShader.use();

	prepareCamera(surfaceShader.program);

	glBindVertexArray(surfaceVAO);
	glPointSize(2);
	glDrawArrays(GL_POINTS, 0, surfaceVertices.size());

	glBindVertexArray(0);
	glUseProgram(0);
}

void drawSurfaceWithEdges() {
	surfaceShader.use();

	prepareCamera(surfaceShader.program);

	glBindVertexArray(surfaceVAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, surfaceVertices.size());

	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(0);
}

void drawSurfaceWithFaces() {
	surfaceShader.use();

	prepareCamera(surfaceShader.program);

	glBindVertexArray(surfaceVAO);
	for(int i = 0; i < surfaceVertices.size(); i += 6) {
		glDrawArrays(GL_LINE_STRIP, i, 6);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void drawAxis() {
	defaultShader.use();

	glBindVertexArray(axisVAO);
	glDrawArrays(GL_LINES, 0, 6);

	glBindVertexArray(0);
	glUseProgram(0);
}

void drawAxis2() {
	surfaceShader.use();

	prepareCamera(surfaceShader.program);

	glBindVertexArray(axisVAO);
	glDrawArrays(GL_LINES, 0, 6);

	glBindVertexArray(0);
	glUseProgram(0);
}

void drawBaseLines() {
	defaultShader.use();

	glBindVertexArray(baseVAO);
	glPointSize(6.0);
	glDrawArrays(GL_POINTS, 0, baseLineVertices.size());
	glDrawArrays(GL_LINE_STRIP, 0, baseLineVertices.size());

	glBindVertexArray(0);
	glUseProgram(0);
}

void drawCurve() {
	defaultShader.use();

	glBindVertexArray(curveVAO);
	glDrawArrays(GL_LINE_STRIP, 0, bezierCurveVertices.size());

	glBindVertexArray(0);
	glUseProgram(0);
}

//----------------- DRAW COMMANDS ---------------------

//----------------- EVENT HANDLERS ---------------------

void mousePosFunc(GLFWwindow* window, double xpos, double ypos) {
	MOUSEX = xpos;
	MOUSEY = ypos;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
	lastX = xpos;
	lastY = ypos;

	if (keys[GLFW_KEY_RIGHT_SHIFT]) {
		camera.processMouseMovement(xoffset, yoffset);
	}
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

void keyPressFunc(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			keys[key] = true;
		} else if (action == GLFW_RELEASE) {
			keys[key] = false;
		}
	}

	if (action == GLFW_PRESS) {
		switch (key) {

		case GLFW_KEY_G:
			camera.position = vec3(1.3f, 0.0f, 0.0f);
			camera.up = vec3(0.0f, 1.0f, 0.0f);
			camera.front = vec3(-1.0f, 0.0f, 0.0f);
			break;

		case GLFW_KEY_H:
			camera.position = vec3(0.0f, 1.3f, 0.0f);
			camera.up = vec3(0.0f, 0.0f, 1.0f);
			camera.front = vec3(0.0f, -1.0f, 0.0f);
			break;

		case GLFW_KEY_J:
			camera.position = vec3(0.0f, 0.0f, 1.3f);
			camera.up = vec3(0.0f, 1.0f, 0.0f);
			camera.front = vec3(0.0f, 0.0f, -1.0f);
			break;

		case GLFW_KEY_K: {
			const double pos = sqrt(1 / 3.0);
			camera.position = vec3(pos, pos, pos);
			camera.up = vec3(0.0f, 1.0f, 0.0f);
			camera.front = vec3(-pos, -pos, -pos);
		}
		break;


		case GLFW_KEY_L:
			camera.reset();
			break;

		case GLFW_KEY_Q:
			if (baseLineVertices.size() > 0) {
				baseLineVertices.pop_back();
				calculateBezier();
			}
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
			changeTexture("res/textures/homogenous.jpg");
			break;

		case GLFW_KEY_2:
			surfaceMode = 't';
			glDeleteTextures(1, &texture);
			changeTexture("res/textures/metal.jpg");
			break;

		case GLFW_KEY_3:
			surfaceMode = 't';
			glDeleteTextures(1, &texture);
			changeTexture("res/textures/natural.jpg");
			break;

		case GLFW_KEY_4:
			glDeleteTextures(1, &texture);
			surfaceMode = 'v';
			break;

		case GLFW_KEY_5:
			glDeleteTextures(1, &texture);
			surfaceMode = 'e';
			break;

		case GLFW_KEY_6:
			glDeleteTextures(1, &texture);
			surfaceMode = 'f';
			break;

		default:
			break;
		}
	}
}

//----------------- EVENT HANDLERS ---------------------

void display() {
	if (isRotated) {
		drawAxis2();

		switch (surfaceMode) {

		case 't':
			drawSurfaceWithTex();
			break;

		case 'v':
			drawSurfaceWithVertices();
			break;

		case 'f':
			drawSurfaceWithFaces();
			break;

		case 'e':
			drawSurfaceWithEdges();
			break;

		}

	} else {
		drawAxis();
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

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Computação Visual", glfwGetPrimaryMonitor(), nullptr);

	if (nullptr == window) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SWIDTH, &SHEIGHT);

	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit()) {
		cout << "Failed to initialize GLEW" << endl;
		return EXIT_FAILURE;
	}

	glfwSetKeyCallback(window, keyPressFunc);
	glfwSetCursorPosCallback(window, mousePosFunc);
	glfwSetMouseButtonCallback(window, mouseClickFunc);

	glViewport(0, 0, SWIDTH, SHEIGHT);

	initGL();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	defaultShader.load("res/shaders/default/vertex.glsl", "res/shaders/default/fragment.glsl");
	textureShader.load("res/shaders/texture/vertex.glsl", "res/shaders/texture/fragment.glsl");
	surfaceShader.load("res/shaders/surface/vertex.glsl", "res/shaders/default/fragment.glsl");

	createAxis();
	createBase();
	createCurve();

	while (!glfwWindowShouldClose(window)) {
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		doMovement();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
