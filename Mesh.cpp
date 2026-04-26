#include <glad/glad.h>
#include <iostream>

#include "Mesh.h"
#include "PoliceCarModel.h"

Mesh::Mesh(const float* vertices, size_t vertexCount, const std::weak_ptr<Shader>& shader, const Transform& transform, const glm::vec3& color, const unsigned int* indices, size_t indexCount) :
    shader(shader),
    transform(transform),
    meshColor(color)
{
    if (vertexCount % 14 == 0 && (vertexCount / 14) > 0 && vertexCount % 11 != 0)
    {
        SetupMesh(vertices, vertexCount, indices, indexCount);
    }
    else
    {
        size_t numVerts = vertexCount / 11;
        std::vector<float> expandedVertices;
        expandedVertices.reserve(numVerts * 14);

        for (size_t i = 0; i < numVerts; ++i) 
        {
            size_t offset = i * 11;
            for (int j = 0; j < 11; ++j) expandedVertices.push_back(vertices[offset + j]);
            expandedVertices.push_back(color.r);
            expandedVertices.push_back(color.g);
            expandedVertices.push_back(color.b);
        }
        SetupMesh(expandedVertices.data(), expandedVertices.size(), indices, indexCount);
    }
}

Mesh::Mesh(const std::weak_ptr<Shader>& shader) :
    shader(shader) {}

Mesh::Mesh(Mesh&& other) noexcept :
    VBO(std::exchange(other.VBO, 0)),
    VAO(std::exchange(other.VAO, 0)),
    EBO(std::exchange(other.EBO, 0)),
    vertexCount(other.vertexCount),
    indexCount(other.indexCount),
    meshColor(other.meshColor),
    transform(std::move(other.transform)),
    texture(std::move(other.texture)),
    shader(other.shader),
    vertexCache(std::move(other.vertexCache)),
    indexCache(std::move(other.indexCache)) {}

Mesh::~Mesh()
{
    // Delete buffers.
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (EBO != 0)
    {
        glDeleteBuffers(1, &EBO);
    }
}

void Mesh::Draw(std::shared_ptr<Shader> customShader) const
{
    std::shared_ptr<Shader> s = customShader;
    if (!s) 
    {
        s = shader.lock(); // Fallback to internal shader
    }

    if (!s)
    {
        std::cerr << "{Mesh::Draw} [ERROR] Shader expired or not set!" << std::endl;
        return;
    }

    s->UseShader();

    // Set the uniform locations.
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(transform.GetModelMatrix())));
    glUniformMatrix4fv(s->GetUniformLocation("modelMatrix"), 1, GL_FALSE, &transform.GetModelMatrix()[0][0]);
    glUniformMatrix3fv(s->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

    DrawMesh(s);
}

void Mesh::DrawRaw(std::shared_ptr<Shader> customShader) const
{
    std::shared_ptr<Shader> s = customShader;
    if (!s)
    {
        s = shader.lock(); // Fallback to internal shader
    }

    if (!s)
    {
        std::cerr << "{Mesh::Draw} [ERROR] Shader expired or not set!" << std::endl;
        return;
    }

    s->UseShader();

    DrawMesh(s);
}

void Mesh::SetupMesh(const float* vertices, size_t vCount, const unsigned int* indices, size_t iCount)
{
    this->vertexCount = vCount;
    this->indexCount = iCount;

    vertexCache.assign(vertices, vertices + vCount);
    if (indices && iCount > 0)
    {
        indexCache.assign(indices, indices + iCount);
    }

    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO to VBO
    glBindVertexArray(VAO);

    // Set VBO data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(float), vertices, GL_STATIC_DRAW);

    // If we have indices
    if (indices != nullptr && iCount > 0) 
    {
        // Generate EBO and bind it
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    }

    // Attributes
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE_BYTES, (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE_BYTES, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texturing
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, STRIDE_BYTES, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Tangents
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, STRIDE_BYTES, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Color
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, STRIDE_BYTES, (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    // Cleanup.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

std::weak_ptr<Shader> Mesh::GetShader() const { return shader; }

void Mesh::SetShader(std::weak_ptr<Shader> newShader) { shader = newShader; }

const Transform& Mesh::GetTransform() const { return transform; }

void Mesh::SetTransform(const Transform& newTransform) { transform = newTransform; }

const glm::vec3& Mesh::GetColor() const { return meshColor; }

void Mesh::SetColor(const glm::vec3& newColor)
{
    meshColor = newColor;

    // Update local cache.
    for (size_t i = 0; i < vertexCache.size(); i += STRIDE)
    {
        vertexCache[i + 11] = newColor.r;
        vertexCache[i + 12] = newColor.g;
        vertexCache[i + 13] = newColor.b;
    }

    // Push to GPU.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCache.size() * sizeof(float), vertexCache.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::SetTexture(std::shared_ptr<Texture> tex) { texture = tex; }

std::vector<float> Mesh::GetVertexData() const { return vertexCache; }

std::vector<unsigned int> Mesh::GetIndexData() const { return indexCache; }

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (EBO != 0)
    {
        glDeleteBuffers(1, &EBO);
    }

    VBO = std::exchange(other.VBO, 0);
    VAO = std::exchange(other.VAO, 0);
    EBO = std::exchange(other.EBO, 0);
    vertexCount = other.vertexCount;
    indexCount = other.indexCount;
    meshColor = other.meshColor;
    transform = std::move(other.transform);
    texture = std::move(other.texture);
    shader = other.shader;
    vertexCache = std::move(other.vertexCache);
    indexCache = std::move(other.indexCache);

    return *this;
}

void Mesh::DrawMesh(std::shared_ptr<Shader>& shader) const
{
    // Bind the VAO.
    glBindVertexArray(VAO);

    // Check if we have a texture
    GLint hasTexLoc = shader->GetUniformLocation("hasTexture");
    if (hasTexLoc != -1)
    {
        if (texture)
        {
            texture->Bind(0);
            glUniform1i(shader->GetUniformLocation(texture->GetUniformName()), 0);
            glUniform1i(hasTexLoc, 1);
        }
        else
        {
            glUniform1i(hasTexLoc, 0);
        }
    }

    // If we have an EBO.
    if (EBO != 0)
    {
        // Use it to draw the mesh.
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    }
    else
    {
        // Otherwise just draw it normally.
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount / STRIDE));
    }

    // Unbind.
    glBindVertexArray(0);
    if (texture && hasTexLoc != -1)
    {
        texture->Unbind();
    }
}
