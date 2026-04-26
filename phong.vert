#version 330 core

layout (location = 0) in vec3 aPos;         // Position coordinates,    3 floats.
layout (location = 1) in vec3 aNormal;      // Normal coordinates,      3 floats.
layout (location = 2) in vec2 aTexCoords;   // Texture coordinates,     2 floats.
layout (location = 3) in vec3 aTangents;    // Tanget values,           3 floats.
layout (location = 4) in vec3 aColor;       // Color values,            3 floats.

uniform mat4 modelMatrix;       // Model matrix.
uniform mat3 normalMatrix;      // Normal matrix.
uniform mat4 projViewMatrix;    // Projection * view Matrix

out vec3 FragPos;       // World space position.
out vec3 Normal;        // Normal vector for lighting.
out vec2 TexCoords;     // Texture coordinates.
out vec3 Tangents;      // Tangets.
out vec3 PassColor;     // Color.

void main()
{
    // Texture coordinates.
    TexCoords = aTexCoords;

    // Color.
    PassColor = aColor;

    // Tangets
    Tangents = normalMatrix * aTangents;

    // Position of fragments.
    FragPos = vec3(modelMatrix * vec4(aPos, 1.0));

    // Normal vector.
    Normal = normalMatrix * aNormal; 

    // Position.
    gl_Position = projViewMatrix * vec4(FragPos, 1.0);
}