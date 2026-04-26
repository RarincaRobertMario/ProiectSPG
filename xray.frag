#version 330 core
out vec4 FragColor;     /// Out frag color.
    
uniform vec3 glowColor = vec3(1.0f, 1.0f, 1.0f); /// Glow color.
uniform float opacity = 1.0f;  /// Opacity.

void main() 
{
    FragColor = vec4(glowColor, opacity);
}