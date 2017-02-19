#ifndef CAMERA_H_
#define CAMERA_H_

#include <vector>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "DataStructures.h"

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVTY = 0.20f;
const GLfloat ZOOM = 45.0f;

class Camera {

public:

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat movementSpeed;
	GLfloat mouseSensitivity;
	GLfloat zoom;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up =
			glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch =
					PITCH) :
						front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(
								SENSITIVTY), zoom(ZOOM) {
		this->position = position;
		this->worldUp = up;
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY,
			GLfloat upZ, GLfloat yaw, GLfloat pitch) :
				front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(
						SENSITIVTY), zoom(ZOOM) {
		this->position = glm::vec3(posX, posY, posZ);
		this->worldUp = glm::vec3(upX, upY, upZ);
		this->yaw = yaw;
		this->pitch = pitch;
		this->updateCameraVectors();
	}

	void reset() {
		this->position = glm::vec3(0.0f, 0.0f, 1.1f);
		this->worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		this->yaw = YAW;
		this->pitch = PITCH;
		front = glm::vec3(0.0f, 0.0f, -1.0f);
		movementSpeed = SPEED;
		mouseSensitivity = SENSITIVTY;
		zoom = 45.0f;
		this->updateCameraVectors();
	}

	glm::mat4 getViewMatrix() {
		return glm::lookAt(this->position, this->position + this->front,
				this->up);
	}

	void processKeyboard(CameraMovement direction, GLfloat deltaTime) {
		GLfloat velocity = this->movementSpeed * deltaTime;

		if (direction == FORWARD) {
			this->position += this->front * velocity;
		}

		if (direction == BACKWARD) {
			this->position -= this->front * velocity;
		}

		if (direction == LEFT) {
			this->position -= this->right * velocity;
		}

		if (direction == RIGHT) {
			this->position += this->right * velocity;
		}
	}

	void processMouseMovement(GLfloat xOffset, GLfloat yOffset,
			GLboolean constrainPitch = true) {
		xOffset *= this->mouseSensitivity;
		yOffset *= this->mouseSensitivity;

		this->yaw += xOffset;
		this->pitch += yOffset;

		if (constrainPitch) {
			if (this->pitch > 89.0f) {
				this->pitch = 89.0f;
			}

			if (this->pitch < -89.0f) {
				this->pitch = -89.0f;
			}
		}

		this->updateCameraVectors();
	}

	void updateCameraVectors() {
		glm::vec3 front;
		front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		front.y = sin(glm::radians(this->pitch));
		front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front = glm::normalize(front);

		this->right = glm::normalize(glm::cross(this->front, this->worldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->up = glm::normalize(glm::cross(this->right, this->front));
	}

};

#endif
