#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inColor;
layout(location = 3) in vec3 normal;

out vec3 color;

uniform mat4 view;
uniform mat4 projection;

void main() {
  vec3 temp;

  temp.x = normal.x * 0.1;
  temp.y = normal.y * 0.1;
  temp.z = normal.z * 0.1;

  vec3 result = position + temp;

	gl_Position = projection * view * vec4(result, 1.0f);
  color = inColor;
}
