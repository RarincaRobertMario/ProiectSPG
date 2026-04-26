#version 330 core
layout (location = 0) in vec3 aPos; // In vertex position

uniform mat4 modelMatrix;       // Model matrix.
uniform mat4 projViewMatrix;    // Projection * view Matrix

void main() 
{
    gl_Position = projViewMatrix * modelMatrix * vec4(aPos, 1.0);
}