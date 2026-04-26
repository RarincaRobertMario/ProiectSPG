#include <glad/glad.h>
#include <string>

#include "SpotLight.h"
#include "Shader.h"

void SpotLight::Apply(const Shader& shader, int index) const
{
    std::string baseName = "spotLights[" + std::to_string(index) + "]";
    glUniform3fv(shader.GetUniformLocation(baseName + ".position"), 1, &base.position[0]);
    glUniform3fv(shader.GetUniformLocation(baseName + ".color"), 1, &base.currentColor[0]);

    glUniform1f(shader.GetUniformLocation(baseName + ".constant"), base.constant);
    glUniform1f(shader.GetUniformLocation(baseName + ".linear"), base.linear);
    glUniform1f(shader.GetUniformLocation(baseName + ".quadratic"), base.quadratic);
    glUniform1f(shader.GetUniformLocation(baseName + ".intensity"), base.intensity);

    glUniform3fv(shader.GetUniformLocation(baseName + ".direction"), 1, &direction[0]);
    glUniform1f(shader.GetUniformLocation(baseName + ".cutOff"), cutOff);
    glUniform1f(shader.GetUniformLocation(baseName + ".outerCutOff"), outerCutOff);

    glUniform1i(shader.GetUniformLocation(baseName + ".shadowMapIdx"), static_cast<int>(shadowMapIdx));
}
