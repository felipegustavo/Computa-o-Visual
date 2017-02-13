#ifndef DATASTRUCTURES_H_
#define DATASTRUCTURES_H_

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct ColorVertex {
	glm::vec3 position;
	glm::vec3 color;

	ColorVertex(const glm::vec3 &position2, const glm::vec3 &color2) {
		position = position2;
		color = color2;
	}

	const ColorVertex & operator=(const ColorVertex &vf) {
		position = vf.position;
		color = vf.color;
		return *this;
	}
};

struct TextureVertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	TextureVertex(const glm::vec3 &position2, const glm::vec3 &color2, const glm::vec2 &texCoord2) {
		position = position2;
		color = color2;
		texCoord = texCoord2;
	}

	const TextureVertex & operator=(const TextureVertex &vf) {
		position = vf.position;
		color = vf.color;
		texCoord = vf.texCoord;
		return *this;
	}
};

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

#endif
