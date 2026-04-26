#pragma once

#ifndef POLICECARMODEL_H_
#define POLICECARMODEL_H_

#include <array>

#include "IRenderable.h"
#include "Transform.h"
#include "PoliceCarModelData.h"
#include "BakePassType.h"
#include "WindowType.h"

/**
* @brief Class for the model of a police car. Does not update.
*/
class PoliceCarModel : public IRenderable 
{
protected:
    std::shared_ptr<PoliceCarModelData> data;    /// Police data. Technically this mangles design patterns, because PoliceCarActive won't share its data, but whatever.
    Transform transform;    /// Transform of the model.

    /**
    * @brief Returns a part with a certain index.
    * 
    * @param index Index of the part.
    * 
    * @return A pointer to the part, or NULLPTR if it doesn't exist.
    */
    PoliceCarPart* GetPart(PoliceCarPartIndex index);

    /**
    * @brief Returns a part witha  certain index.
    * 
    * @param index Index of the part.
    * 
    * @return A constant pointer to the part, or NULLPTR if it doesn't exist.
    */
    const PoliceCarPart* GetPart(PoliceCarPartIndex index) const;

    /**
    * @brief Creates a part.
    * 
    * @param mesh Unique pointer to the mesh.
    * @param type The mesh type.
    * @param index Index of the part, if any.
    * @param offset Offset of the part.
    * 
    * @return Reference to the newly-created part, for any other modifications.
    */
    PoliceCarPart& CreatePart(
        std::unique_ptr<Mesh> mesh,
        PoliceCarMeshType type,
        PoliceCarPartIndex index,
        const glm::vec3& offset = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Initialises the police car's parts.
    *
    * The caller, or child classes, should call this, in order to use virtual calls for any override AddXPart(...)
    *
    * @param shader Weak pointer to the shader.
    */
    virtual void InitParts(const std::weak_ptr<Shader>& shader);

    /**
    * @brief Creates the main hollow body of the car.
    * 
    * @param shader Weak pointer to the shader.
    * @param color Color of the body.
    * @param pos Position of the body.
    * @param scale Scale of the body.
    * @param rot Rotation of the body.
    */
    virtual void CreateBody(
        const std::weak_ptr<Shader>& shader, 
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the cabin of the car.
    *
    * @param shader Weak pointer to the shader.
    * @param alpha Alpha tranparency of the window.
    * @param bodyColor Color of the body parts.
    * @param windowColor Color of the window parts.
    * @param pos Position of the cabin.
    * @param scale Scale of the cabin.
    * @param rot Rotation of the cabin.
    */
    virtual void CreateCabinMain(
        const std::weak_ptr<Shader>& shader,
        uint8_t alpha = 255,
        const glm::vec3& bodyColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& windowColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a seat.
    * 
    * @param shader Shader used.
    * @param color Color of the seat
    * @param pos Position of the seat.
    * @param scale Scale of the seat.
    * @param rot Rotation of the seat.
    */
    virtual void CreateSeat(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the wheels
    *
    * @param shader Shader used.
    * @param color Color of the wheels
    * @param pos Position of the wheels.
    * @param scale Scale of the wheels.
    * @param rot Rotation of the wheels.
    */
    virtual void CreateWheels(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the handles
    *
    * @param shader Shader used.
    * @param color Color of the handles
    * @param pos Position of the handles.
    * @param scale Scale of the handles.
    * @param rot Rotation of the handles.
    */
    virtual void CreateHandles(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the bumpers
    *
    * @param shader Shader used.
    * @param color Color of the bumpers
    * @param pos Position of the bumpers.
    * @param scale Scale of the bumpers.
    * @param rot Rotation of the bumpers.
    */
    virtual void CreateBumpers(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the license plates
    *
    * @param shader Shader used.
    * @param color Color of the plates
    * @param pos Position of the plates.
    * @param scale Scale of the plates.
    * @param rot Rotation of the plates.
    */
    virtual void CreatePlates(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the side mirrors
    *
    * @param shader Shader used.
    * @param bodyColor Color of the mirrors' body.
    * @param windowColor Color of the mirrors' mirror.
    * @param pos Position of the mirrors.
    * @param scale Scale of the mirrors.
    * @param rot Rotation of the mirrors.
    */
    virtual void CreateMirrors(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& bodyColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& mirrorColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the grille.
    *
    * @param shader Shader used.
    * @param grileBgColor Color of the grille's background.
    * @param grileColor Color of the grilles.
    * @param pos Position of the grille.
    * @param scale Scale of the grille.
    * @param rot Rotation of the grille.
    */
    virtual void CreateGrille(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& grilleBgColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& grilleColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the cabin side.
    *
    * @param shader Shader used.
    * @param bodyColor Color of the cabin's body.
    * @param windowColor Color of the cabin's windows.
    * @param pos Position of the cabin's side.
    * @param scale Scale of the cabin's side.
    * @param rot Rotation of the cabin's side.
    */
    virtual void CreateCabinSide(
        const std::weak_ptr<Shader>& shader,
        FacePosition side,
        uint8_t alpha,
        const glm::vec3& bodyColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& windowColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the lightbar.
    *
    * @param shader Shader used.
    * @param boxColor Color of the lightbar's box.
    * @param pos Position of the lightbar.
    * @param scale Scale of the lightbar.
    * @param rot Rotation of the lightbar.
    */
    virtual void CreateLightBar(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& boxColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the front lights.
    *
    * @param shader Shader used.
    * @param shortBeamColor Color of the short beam.
    * @param highBeamColor Color of the high beam.
    * @param blinkerColor Color of the blinkers.
    * @param pos Position of the front lights.
    * @param scale Scale of the front lights.
    * @param rot Rotation of the front lights.
    */
    virtual void CreateFrontLights(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& shortBeamColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& highBeamColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& blinkerColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the back lights.
    *
    * @param shader Shader used.
    * @param brakeLightColor Color of the brake lights.
    * @param reverLightColor Color of the reverse lights.
    * @param blinkerColor Color of the blinkers.
    * @param pos Position of the back lights.
    * @param scale Scale of the back lights.
    * @param rot Rotation of the back lights.
    */
    virtual void CreateBackLights(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& brakeLightColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& reverLightColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& blinkerColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a steering wheel
    * 
    * @param shader Shader used.
    * @param color Color of the steering wheel
    * @param pos Position of the steering wheel.
    * @param scale Scale of the steering wheel.
    * @param rot Rotation of the steering wheel.
    */
    virtual void CreateSteeringWheel(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a body part.
    *
    * @param shader Weak pointer to the shader.
    * @param type Type of mesh to add.
    * @param index Index of the part, if any.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    virtual void AddBodyPart(
        const std::weak_ptr<Shader>& shader,
        PoliceCarMeshType type,
        PoliceCarPartIndex index = PoliceCarPartIndex::None,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a spotlight part.
    *
    * @param shader Weak pointer to the shader.
    * @param lType Type of light to add.
    * @param side What side the light is located on.
    * @param index Index of the part, if any.
    * @param shadowMapIndex Index of this part's shadowMap, as defined in the fragment shader, if any.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    virtual void AddSpotLightPart(
        const std::weak_ptr<Shader>& shader,
        PoliceCarLightType lType,
        FacePosition side,
        PoliceCarPartIndex index = PoliceCarPartIndex::None,
        ShadowMapIndex shadowMapindex = ShadowMapIndex::None,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
     * @brief Adds a point light.
     *
     * @param shader Weak pointer to the shader.
     * @param lType Type of light to add.
     * @param side What side the light is located on.
     * @param index Index of the part, if any.
     * @param shadowCubeIndex Index of this part's shadow cube, as defined in the fragment shader, if any.
     * @param color Color of the mesh.
     * @param pos Position of the mesh.
     * @param scale Scale of the mesh.
     * @param rot Rotation of the mesh.
     */
    virtual void AddPointLightPart(
        const std::weak_ptr<Shader>& shader,
        PoliceCarLightType lType,
        FacePosition side,
        PoliceCarPartIndex index = PoliceCarPartIndex::None,
        ShadowCubeIndex shadowCubeIndex = ShadowCubeIndex::None,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a wheel part.
    *
    * @param shader Weak pointer to the shader.
    * @param index Index of the part, if any.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    virtual void AddWheel(
        const std::weak_ptr<Shader>& shader,
        PoliceCarPartIndex index,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a window to the car.
    * 
    * @param shader Weak pointer to the shader.
    * @param type Type of window.
    * @param index Index of the part, if any.
    * @param alpha Alpha (transparency) of the window. 255 for opaque, 0 for fully transparent.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    virtual void AddWindowPart(
        const std::weak_ptr<Shader>& shader,
        WindowType type,
        PoliceCarPartIndex index,
        uint8_t alpha = 255,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );


    /**
    * @brief Bakes the static car parts into a single mesh.
    *
    * @param shader Weak pointer to the shader used.
    */
    virtual void Bake(const std::weak_ptr<Shader>& shader);

    /**
    * @brief Performs a bake pass.
    * 
    * @param shader Weak pointer to the shader.
    * @param passType The bake pass type.
    * 
    * @return A unique pointer to the baked mesh created.
    */
    virtual std::unique_ptr<Mesh> PerformBakePass(
        const std::weak_ptr<Shader>& shader,
        BakePassType passType
    );

    /**
    * @brief Subclass specific draw method.
    * 
    * This function should be overloaded instead.
    * 
    * @param customShader Custom shader to use. Nullptr to use meshes' own shaders.
    */
    virtual void OnDraw(std::shared_ptr<Shader> customShader = nullptr) const;

public:

    /**
    * @brief Constructor.
    *
    * Do note that that you cannot set the overral color of the car.
    *
    * @param shader Weak pointer to the shader used.
    * @param transform Transformation structure for the car. Added ontop of the mesh ones.
    */
    PoliceCarModel(const std::weak_ptr<Shader>& shader, const Transform& t = {});

    /**
    * @brief Constructor.
    *
    * Do note that that you cannot set the overral color of the car.
    *
    * @param sharedData Shared pointer to the shared data, assumes the data has already been created.
    * @param transform Transformation structure for the car. Added ontop of the mesh ones.
    */
    PoliceCarModel(const std::shared_ptr<PoliceCarModelData>& sharedData, const Transform& t = {});

    /**
    * @brief 'Constructor'.
    *
    * This isn't a classic constructor, more so a builder method, where we build the data and the object is then 'discarded'.
    * 
    * @param shader Weak pointer to the shader.
    * @param sharedData Shared pointer to the shared data.
    */
    PoliceCarModel(const std::weak_ptr<Shader>& shader, const std::shared_ptr<PoliceCarModelData>& sharedData);

    /**
    * @brief Default destructor.
    */
    virtual ~PoliceCarModel() = default;

    /**
    * @brief Creates an instance of the model with shared data.
    * 
    * @param transform Individual ('unique') transform for the newly created model.
    * 
    * @return A shared pointer to the newly created model.
    */
    std::shared_ptr<PoliceCarModel> CreateInstance(const Transform& t = {});

    /**
    * @brief Draws the police car.
    *
    * 'OnDraw' should be overloaded instead.
    * 
    * @param customShader Custom shader to use. Nullptr to use meshes' own shaders.
    */
    virtual void Draw(std::shared_ptr<Shader> customShader = nullptr) const override;

    /**
    * @brief Sets the transform of the model.
    * 
    * @param newTransform New transform of the model.
    */
    virtual void SetTransform(const Transform& newTransform);

    /**
    * @brief Returns the transform of the model.
    * 
    * @return Constant reference to the model's transofmr.
    */
    virtual const Transform& GetTransform() const;

    /**
    * @brief Returns the transform of the model.
    *
    * @return A copy of the model's transofmr.
    */
    virtual Transform GetTransform();
};

#endif // POLICECARMODEL_H_