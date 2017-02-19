#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inTexCoord;

out vec2 texCoord;

uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * vec4(inPosition, 1.0f);
    texCoord = vec2(inTexCoord.x, 1.0 - inTexCoord.y);
}