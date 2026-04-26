#include <glad/glad.h>

#include "DirLight.h"
#include "Shader.h"

void DirLight::Apply(const Shader& shader) const
{
    glUniform3fv(shader.GetUniformLocation(name + ".direction"), 1, &direction[0]);
    glUniform3fv(shader.GetUniformLocation(name + ".color"), 1, &color[0]);
    glUniform1f(shader.GetUniformLocation(name + ".intensity"), intensity);
}
