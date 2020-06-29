#version 330 core
#define step 5.0f

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 position;

void main()
{
    gl_Position = projection * vec4(aPos, 1.0);
}