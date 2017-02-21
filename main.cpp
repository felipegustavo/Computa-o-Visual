#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <utility>
#include <cmath>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/normal.hpp>
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

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

Shader defaultShader;
Shader textureShader;
Shader surfaceShader;
Shader ctrlShader;
Shader lampShader;

GLuint surfaceVAO;
GLuint surfaceVBO;

GLuint baseVAO;
GLuint baseVBO;

GLuint curveVAO;
GLuint curveVBO;

GLuint axisVAO;
GLuint axisVBO;

GLuint lampVAO;
GLuint lampVBO;

GLuint specularTexture;
GLuint diffuseTexture;

glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f, 0.2f, 2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f, 2.0f, -12.0f),
	glm::vec3(0.0f, 0.0f, -3.0f)
};

vector<ColorVertex> baseLineVertices;
vector<ColorVertex> bezierCurveVertices;
vector<TextureVertex> surfaceVertices;

bool isRotated = false;
char surfaceMode = 't';
bool isCtrl = false;
bool useViewport = false;

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

			curveVec.push_back(TextureVertex(rvec, vec3(1.0f, 1.0f, 1.0f), vec2(0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)));
		}
		surfVec.push_back(curveVec);
	}

	surfaceVertices.clear();

	for (int i = 0; i < surfVec.size() - 1; i++) {
		for (int j = 0; j < surfVec[i].size() - 1; j++) {
			vec3 normal = triangleNormal(surfVec[i][j + 1].position, surfVec[i + 1][j + 1].position, surfVec[i + 1][j].position);

			surfaceVertices.push_back(TextureVertex(surfVec[i][j + 1].position, surfVec[i][j + 1].color, vec2(0.0f, 0.0f), normal));
			surfaceVertices.push_back(TextureVertex(surfVec[i + 1][j + 1].position, surfVec[i + 1][j + 1].color, vec2(1.0f, 0.0f), normal));
			surfaceVertices.push_back(TextureVertex(surfVec[i + 1][j].position,	surfVec[i + 1][j].color, vec2(1.0f, 1.0f), normal));

			normal = triangleNormal(surfVec[i + 1][j].position, surfVec[i][j].position, surfVec[i][j + 1].position);

			surfaceVertices.push_back(TextureVertex(surfVec[i + 1][j].position, surfVec[i + 1][j].color, vec2(1.0f, 1.0f), normal));
			surfaceVertices.push_back(TextureVertex(surfVec[i][j].position, surfVec[i][j].color, vec2(0.0f, 1.0f), normal));
			surfaceVertices.push_back(TextureVertex(surfVec[i][j + 1].position, surfVec[i][j + 1].color, vec2(0.0f, 0.0f), normal));
		}
	}

	const int p = surfVec.size() - 1;
	for (int j = 0; j < surfVec.back().size() - 1; j++) {
		vec3 normal = triangleNormal(surfVec[p][j + 1].position, surfVec[0][j + 1].position, surfVec[0][j].position);

		surfaceVertices.push_back(TextureVertex(surfVec[p][j + 1].position, surfVec[p][j + 1].color, vec2(0.0f, 0.0f), normal));
		surfaceVertices.push_back(TextureVertex(surfVec[0][j + 1].position, surfVec[0][j + 1].color, vec2(1.0f, 0.0f), normal));
		surfaceVertices.push_back(TextureVertex(surfVec[0][j].position, surfVec[0][j].color, vec2(1.0f, 1.0f), normal));

		normal = triangleNormal(surfVec[0][j].position, surfVec[p][j].position, surfVec[p][j + 1].position);

		surfaceVertices.push_back(TextureVertex(surfVec[0][j].position, surfVec[0][j].color, vec2(1.0f, 1.0f), normal));
		surfaceVertices.push_back(TextureVertex(surfVec[p][j].position, surfVec[p][j].color, vec2(0.0f, 1.0f), normal));
		surfaceVertices.push_back(TextureVertex(surfVec[p][j + 1].position,	surfVec[p][j + 1].color, vec2(0.0f, 0.0f), normal));
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

void applyTexture(const char* diffuseTex, const char* specularTex) {
	glGenTextures(1, &diffuseTexture);
	glGenTextures(1, &specularTexture);

	int imageWidth, imageHeight;
	unsigned char *image;

	image = SOIL_load_image(diffuseTex, &imageWidth, &imageHeight, 0, SOIL_LOAD_RGB);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	image = SOIL_load_image(specularTex, &imageWidth, &imageHeight, 0, SOIL_LOAD_RGB);
	glBindTexture(GL_TEXTURE_2D, specularTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void createLamp() {
	glGenVertexArrays(1, &lampVAO);
	glGenBuffers(1, &lampVBO);

	glBindVertexArray(lampVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lampVBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);

	const GLfloat lampCube[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(lampCube), lampCube, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void createSurface() {
	glGenVertexArrays(1, &surfaceVAO);
	glGenBuffers(1, &surfaceVBO);

	glBindVertexArray(surfaceVAO);

	glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);
	glBufferData(GL_ARRAY_BUFFER,
	sizeof(TextureVertex) * surfaceVertices.size(), &surfaceVertices[0],
	GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*) 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*) 12);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*) 24);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TextureVertex), (void*) 32);

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

	const ColorVertex vertices[] = {
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

	glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertex) * bezierCurveVertices.size(), &bezierCurveVertices[0], GL_STREAM_DRAW);

	glBindVertexArray(0);
}

//----------------- CREATE AND UPDATE ELEMENTS--------

//----------------- DRAW COMMANDS ---------------------

void drawSurfaceWithTex() {
	textureShader.use();

	GLint viewPosLoc = glGetUniformLocation(textureShader.program, "viewPos");
	glUniform3f(viewPosLoc, camera.position.x, camera.position.y, camera.position.z);
	glUniform1f(glGetUniformLocation(textureShader.program, "material.shininess"), 32.0f);

	glUniform3f(glGetUniformLocation(textureShader.program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
	glUniform3f(glGetUniformLocation(textureShader.program, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(textureShader.program, "dirLight.diffuse"), 0.4f, 0.4f, 0.4f);
	glUniform3f(glGetUniformLocation(textureShader.program, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[0].diffuse"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[0].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[0].quadratic"), 0.032f);

	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[1].diffuse"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[1].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[1].quadratic"), 0.032f);

	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[2].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[2].diffuse"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[2].specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[2].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[2].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[2].quadratic"), 0.032f);

	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[3].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[3].diffuse"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(textureShader.program, "pointLights[3].specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[3].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[3].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(textureShader.program, "pointLights[3].quadratic"), 0.032f);

	glUniform3f(glGetUniformLocation(textureShader.program, "spotLight.position"), camera.position.x, camera.position.y, camera.position.z);
	glUniform3f(glGetUniformLocation(textureShader.program, "spotLight.direction"), camera.front.x, camera.front.y, camera.front.z);
	glUniform3f(glGetUniformLocation(textureShader.program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(textureShader.program, "spotLight.diffuse"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(textureShader.program, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(textureShader.program, "spotLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(textureShader.program, "spotLight.quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(textureShader.program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(textureShader.program, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

	mat4 projection = perspective(camera.zoom, (GLfloat) SWIDTH / (GLfloat) SHEIGHT, 0.1f, 1000.0f);
	mat4 view = camera.getViewMatrix();
	mat4 model = mat4();

	GLint modelLoc = glGetUniformLocation(textureShader.program, "model");
	GLint viewLoc = glGetUniformLocation(textureShader.program, "view");
	GLint projLoc = glGetUniformLocation(textureShader.program, "projection");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularTexture);

	glBindVertexArray(surfaceVAO);
	glDrawArrays(GL_TRIANGLES, 0, surfaceVertices.size());
	glBindVertexArray(0);

	lampShader.use();
	modelLoc = glGetUniformLocation(lampShader.program, "model");
	viewLoc = glGetUniformLocation(lampShader.program, "view");
	projLoc = glGetUniformLocation(lampShader.program, "projection");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	model = glm::mat4();
	model = glm::translate(model, lightPos);
	model = glm::scale(model, glm::vec3(0.1f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(lampVAO);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	for (GLuint i = 0; i < 4; i++) {
		model = glm::mat4();
		model = glm::translate(model, pointLightPositions[i]);
		model = glm::scale(model, glm::vec3(0.1f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

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

void drawCtrlPoints() {
	ctrlShader.use();

	prepareCamera(ctrlShader.program);

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

void setCameraToX() {
	camera.position = vec3(1.3f, 0.0f, 0.0f);
	camera.up = vec3(0.0f, 1.0f, 0.0f);
	camera.front = vec3(-1.0f, 0.0f, 0.0f);
}

void setCameraToY() {
	camera.position = vec3(0.0f, 1.3f, 0.0f);
	camera.up = vec3(0.0f, 0.0f, 1.0f);
	camera.front = vec3(0.0f, -1.0f, 0.0f);
}

void setCameraToZ() {
	camera.position = vec3(0.0f, 0.0f, 1.3f);
	camera.up = vec3(0.0f, 1.0f, 0.0f);
	camera.front = vec3(0.0f, 0.0f, -1.0f);
}

void setCameraToIso() {
	const double pos = sqrt(1 / 3.0);
	camera.position = vec3(pos, pos, pos);
	camera.up = vec3(0.0f, 1.0f, 0.0f);
	camera.front = vec3(-pos, -pos, -pos);
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
			setCameraToX();
			break;

		case GLFW_KEY_H:
			setCameraToY();
			break;

		case GLFW_KEY_J:
			setCameraToZ();
			break;

		case GLFW_KEY_K:
			setCameraToIso();
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

				camera.reset();
				firstMouse = true;

				surfaceMode = 't';
				glDeleteTextures(1, &diffuseTexture);
				glDeleteTextures(1, &specularTexture);
				applyTexture("res/textures/red-diffuse.png", "res/textures/red-specular.png");

				calculateSurfeceVertices('x');
				createSurface();
			}
			break;

		case GLFW_KEY_Y:
			if (bezierCurveVertices.size() > 0) {
				isRotated = true;

				camera.reset();
				firstMouse = true;

				surfaceMode = 't';
				glDeleteTextures(1, &diffuseTexture);
				glDeleteTextures(1, &specularTexture);
				applyTexture("res/textures/red-diffuse.png", "res/textures/red-specular.png");

				calculateSurfeceVertices('y');
				createSurface();
			}
			break;

		case GLFW_KEY_E:
			baseLineVertices.clear();
			bezierCurveVertices.clear();
			surfaceVertices.clear();

			updatePoints();

			isRotated = false;
			isCtrl = false;
			firstMouse = true;
			break;

		case GLFW_KEY_1:
			surfaceMode = 't';
			glDeleteTextures(1, &diffuseTexture);
			glDeleteTextures(1, &specularTexture);
			applyTexture("res/textures/red-diffuse.png", "res/textures/red-specular.png");
			break;

		case GLFW_KEY_2:
			surfaceMode = 't';
			glDeleteTextures(1, &diffuseTexture);
			glDeleteTextures(1, &specularTexture);
			applyTexture("res/textures/metal-diffuse.png", "res/textures/metal-specular.png");
			break;

		case GLFW_KEY_3:
			surfaceMode = 't';
			glDeleteTextures(1, &diffuseTexture);
			glDeleteTextures(1, &specularTexture);
			applyTexture("res/textures/wood-diffuse.png", "res/textures/wood-specular.png");
			break;

		case GLFW_KEY_4:
			glDeleteTextures(1, &diffuseTexture);
			glDeleteTextures(1, &specularTexture);
			surfaceMode = 'v';
			break;

		case GLFW_KEY_5:
			glDeleteTextures(1, &diffuseTexture);
			glDeleteTextures(1, &specularTexture);
			surfaceMode = 'e';
			break;

		case GLFW_KEY_6:
			glDeleteTextures(1, &diffuseTexture);
			glDeleteTextures(1, &specularTexture);
			surfaceMode = 'f';
			break;

		case GLFW_KEY_7:
			isCtrl = !isCtrl;
			break;

		case GLFW_KEY_8:
			useViewport = !useViewport;
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

		if (isCtrl) {
			drawCtrlPoints();
		}

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

void display2() {
	if (!isRotated) {
		return;
	}

	glViewport(0, 360, 640, 360);
	setCameraToX();
	display();

	glViewport(640, 360, 640, 360);
	setCameraToY();
	display();

	glViewport(0, 0, 640, 360);
	setCameraToZ();
	display();

	glViewport(640, 0, 640, 360);
	setCameraToIso();
	display();
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

	defaultShader.load("res/shaders/default/default.vert", "res/shaders/default/default.frag");
	textureShader.load("res/shaders/texture/texture.vert", "res/shaders/texture/texture.frag");
	surfaceShader.load("res/shaders/surface/surface.vert", "res/shaders/default/default.frag");
	lampShader.load("res/shaders/lamp/lamp.vert", "res/shaders/lamp/lamp.frag");
	ctrlShader.load("res/shaders/ctrl/ctrl.vert", "res/shaders/default/default.frag");

	createAxis();
	createBase();
	createCurve();
	createLamp();

	textureShader.use();
	glUniform1i(glGetUniformLocation(textureShader.program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(textureShader.program, "material.specular"), 1);
	glUseProgram(0);

	while (!glfwWindowShouldClose(window)) {
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		doMovement();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (useViewport) {
			display2();
		} else {
			glViewport(0, 0, 1280, 720);
			camera.reset();
			display();
		}

		glfwSwapBuffers(window);
	}

	glDeleteBuffers(1, &axisVBO);
	glDeleteBuffers(1, &baseVBO);
	glDeleteBuffers(1, &curveVBO);
	glDeleteBuffers(1, &surfaceVBO);
	glDeleteBuffers(1, &lampVBO);

	glDeleteVertexArrays(1, &axisVAO);
	glDeleteVertexArrays(1, &baseVAO);
	glDeleteVertexArrays(1, &curveVAO);
	glDeleteVertexArrays(1, &surfaceVAO);
	glDeleteVertexArrays(1, &lampVAO);

	glDeleteTextures(1, &diffuseTexture);
	glDeleteTextures(1, &specularTexture);

	glfwTerminate();

	return EXIT_SUCCESS;
}
