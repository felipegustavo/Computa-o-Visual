#version 330 core

in vec3 color;
in vec2 texCoord;

out vec4 outColor;

uniform sampler2D outTex;

void main() {
	outColor = texture(ourTexture, texCoord);
}