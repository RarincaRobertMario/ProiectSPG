#include <string>
#include <glad/glad.h>

#include "PointLight.h"
#include "Shader.h"

void PointLight::Apply(const Shader& shader, int index) const
{
    std::string baseName = "pointLights[" + std::to_string(index) + "]";
    glUniform3fv(shader.GetUniformLocation(baseName + ".position"), 1, &base.position[0]);
    glUniform3fv(shader.GetUniformLocation(baseName + ".color"), 1, &base.currentColor[0]);

    glUniform1f(shader.GetUniformLocation(baseName + ".constant"), base.constant);
    glUniform1f(shader.GetUniformLocation(baseName + ".linear"), base.linear);
    glUniform1f(shader.GetUniformLocation(baseName + ".quadratic"), base.quadratic);
    glUniform1f(shader.GetUniformLocation(baseName + ".intensity"), base.intensity);

    glUniform1i(shader.GetUniformLocation(baseName + ".shadowCubeIndex"), static_cast<int>(shadowCubeIdx));
}