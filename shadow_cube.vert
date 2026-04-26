#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position.

uniform mat4 modelMatrix; // Model matrix.

void main()
{
    gl_Position = modelMatrix * vec4(aPos, 1.0);
}