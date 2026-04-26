#pragma once

#ifndef MESH_H_
#define MESH_H_

#include <string>
#include <memory>

#include "IRenderable.h"
#include "Shader.h"
#include "Transform.h"
#include "Texture.h"

struct vec3;

// Number of elements in a vertex (3 pos, 3 norm, 2 texture, 3 tangents, 3 colors)
constexpr int STRIDE = 14;
constexpr int STRIDE_BYTES = STRIDE * sizeof(float);

class Mesh : public IRenderable
{
protected:
    std::vector<float> vertexCache;         /// CPU-side copy of vertex data
    std::vector<unsigned int> indexCache;   /// CPU-side copy of index data
    unsigned int VAO = 0;           /// Vertex Array Object; Holds the vertex state information.
    unsigned int VBO = 0;           /// Vertex Buffer Object; Holds the vertexes attributes (positions, normals, texture coords, tangents)        
    unsigned int EBO = 0;           /// Element Buffer Object; holds the indexes of vertexes.
    size_t vertexCount = 0;         /// Vertex count for the VBO.
    size_t indexCount = 0;          /// Index count for the EBO.
    std::weak_ptr<Shader> shader;   /// Weak pointer to the shader used.
    std::shared_ptr<Texture> texture;   /// Shared pointer of texture.
    Transform transform = {};       /// Transform matrix.
    glm::vec3 meshColor = { 0.0f, 0.0f, 0.0f };   /// Color of the mesh.

    // No copying.
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    /**
    * @brief Draws the actual mesh.
    * 
    * @param shader Shared point to the shader.
    */
    virtual void DrawMesh(std::shared_ptr<Shader>& shader) const;

public:

    /**
     * @brief Construct a Mesh with vertices and optional indices.
     * 
     * DOES NOT OWN THE POINTERS!
     * 
     * @param vertices Pointer to array of floats.
     * @param vertexCount Number of vertices (floats).
     * @param shader Weak pointer to the shader to use for this mesh.
     * @param transform Optional transform structure. Leave blank for no transformation.
     * @param color Color of the mesh. Blank for 'black'
     * @param indices Optional pointer to indices (EBO), nullptr if not used.
     * @param indexCount Number of indices, ignored if indices is nullptr.
     */
    Mesh(
        const float* vertices, 
        size_t vertexCount, 
        const std::weak_ptr<Shader>& shader,
        const Transform& transform = {},
        const glm::vec3& baseColor = {0.0f, 0.0f, 0.0f},
        const unsigned int* indices = nullptr, 
        size_t indexCount = 0
    );

    /**
    * @brief Constructor.
    * 
    * @param shader Weak pointer to shader.
    */
    Mesh(const std::weak_ptr<Shader>& shader);

    /**
    * @brief Move constructor.
    */
    Mesh(Mesh&& other) noexcept;

    /**
     * @brief Destructor frees GPU resources.
     */
    virtual ~Mesh();

    /**
     * @brief Draws the mesh.
     * 
     * @param customShader Custom shader to use. Nullptr to use normal shader.
     */
    virtual void Draw(std::shared_ptr<Shader> customShader = nullptr) const override;

    /**
    * @brief Draws the mesh without setting the model matrix. This assumes that the model matrix has already been set.
    * 
    * @param customShader Custom shader to use. Nullptr to use normal shader.
    */
    virtual void DrawRaw(std::shared_ptr<Shader> customShader = nullptr) const;

    /**
    * @brief Sets up the mesh if not done so already.
    * 
    * @param vertices Vector of vertices
    * @param vCount Count of vertices
    * @param indices Vector of indices
    * @param iCount Count of indices
    */
    virtual void SetupMesh(const float* vertices, size_t vCount, const unsigned int* indices, size_t iCount);

    /**
     * @brief Get the shader used by this mesh.
     * 
     * @return A weak pointer to the shader.
     */
    virtual std::weak_ptr<Shader> GetShader() const;

    /**
     * @brief Set a new shader for this mesh.
     * 
     * @param newShader Weak pointer to the shader.
     */
    virtual void SetShader(std::weak_ptr<Shader> newShader);

    /**
    * @brief Returns the transformation of the mesh.
    * 
    * @return Constant reference to the transformation of the mesh.
    */
    virtual const Transform& GetTransform() const;

    /**
    * @brief Sets the transformation of the mesh.
    * 
    * @param newTransform The new transformation of the mesh.
    */
    virtual void SetTransform(const Transform& newTransform);

    /**
    * @brief Returns the color of the mesh.
    *
    * @return Constant reference to the color of the mesh.
    */
    virtual const glm::vec3& GetColor() const;

    /**
    * @brief Sets the color of the mesh.
    *
    * \/!\ CALLS THE GPU TO SET THE DATA. USE WISELY /!\
    * 
    * @param newColor The new color of the mesh.
    */
    virtual void SetColor(const glm::vec3& newColor);

    /**
    * @brief Sets the texture of the mesh.
    * 
    * @param tex Shared pointer to the texture.
    */
    virtual void SetTexture(std::shared_ptr<Texture> tex);

    /**
    * @brief Returns the vertex data of the mesh.
    * 
    * @return A vector of floats representing the vertex data.
    */
    virtual std::vector<float> GetVertexData() const;

    /**
    * @brief Returns the index data of the mesh.
    *
    * @return A vector of unsigned ints representing the vertex indice, or an empty vector if there is no EBO.
    */
    virtual std::vector<unsigned int> GetIndexData() const;

    /**
    * @brief Move operator.
    */
    Mesh& operator=(Mesh&& other) noexcept;
};

#endif // MESH_H_