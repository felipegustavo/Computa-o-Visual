#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

out vec3 color;

uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * vec4(inPosition, 1);
	color = inColor;
}