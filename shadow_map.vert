#version 330 core
layout (location = 0) in vec3 aPos; // Input position of vertex.

uniform mat4 lightSpaceMatrix;  // Light space matrix (~perspective of the light on the scene)
uniform mat4 modelMatrix;       // Model matrix.

void main() 
{
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(aPos, 1.0);
}