#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

#include "PoliceCarModel.h"
#include "CylinderMesh.h"
#include "CubeMesh.h"
#include "BakePassType.h"
#include "WedgeMesh.h"
#include "CityModel.h"
#include "MeshCache.h"
#include "CivilianCarActive.h"

PoliceCarModel::PoliceCarModel(const std::weak_ptr<Shader>& shader, const Transform& t) :
    transform(t) 
{
    data = std::make_shared<PoliceCarModelData>();
}

PoliceCarModel::PoliceCarModel(const std::shared_ptr<PoliceCarModelData>& sharedData, const Transform& t) :
    data(sharedData), 
    transform(t) {}

PoliceCarModel::PoliceCarModel(const std::weak_ptr<Shader>& shader, const std::shared_ptr<PoliceCarModelData>& sharedData) :
    data(sharedData)
{
    InitParts(shader);
    Bake(shader);
}

void PoliceCarModel::Draw(std::shared_ptr<Shader> customShader) const
{
    glm::mat4 carMatrix = transform.GetModelMatrix();
    auto activeShader = customShader ? customShader : data->bakedBodyMesh->GetShader().lock();

    if (!activeShader)
    {
        std::cerr << "{PoliceCarModel::Draw} ERROR: Custom shader is NULLPTR and self-shader is NULLPTR or not set." << std::endl;
        return;
    }

    activeShader->UseShader();

    glUniformMatrix4fv(activeShader->GetUniformLocation("modelMatrix"), 1, GL_FALSE, &carMatrix[0][0]);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(carMatrix)));
    glUniformMatrix3fv(activeShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

    glUniform1i(activeShader->GetUniformLocation("isLightMesh"), 0);
    data->bakedBodyMesh->DrawRaw(customShader);

    if (data->bakedAlphaMesh)
    {
        glEnable(GL_BLEND);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        data->bakedAlphaMesh->DrawRaw(customShader);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    OnDraw(customShader);
}

const Transform& PoliceCarModel::GetTransform() const { return transform; }

Transform PoliceCarModel::GetTransform() { return transform; }

void PoliceCarModel::SetTransform(const Transform& newTransform) { transform = newTransform; }

PoliceCarPart* PoliceCarModel::GetPart(PoliceCarPartIndex index)
{
    auto it = data->carPartIndex[static_cast<size_t>(index)];
    if (it == -1 || static_cast<size_t>(it) >= data->parts.size())
    {
        return nullptr;
    }
    return &data->parts[it];
}

const PoliceCarPart* PoliceCarModel::GetPart(PoliceCarPartIndex index) const
{
    auto it = data->carPartIndex[static_cast<size_t>(index)];
    if (it == -1 || static_cast<size_t>(it) >= data->parts.size())
    {
        return nullptr;
    }
    return &data->parts[it];
}

PoliceCarPart& PoliceCarModel::CreatePart(std::unique_ptr<Mesh> mesh, PoliceCarMeshType type, PoliceCarPartIndex index, const glm::vec3& offset)
{
    PoliceCarPart part;
    part.meshData = std::make_unique<PoliceCarMesh>();
    part.meshData->mesh = std::move(mesh);
    part.meshData->type = type;
    part.meshData->offset = offset;

    data->carPartIndex[static_cast<size_t>(index)] = data->parts.size();
    data->parts.push_back(std::move(part));
    return data->parts.back();
}

void PoliceCarModel::InitParts(const std::weak_ptr<Shader>& shader)
{
    /*
        Unless specified in another way, looking at the front of the car:
        Right -> +x
        Left -> -x
        Up / Down -> +/-y
        Forwards (towards the camera) -> +z
        Backwards (away from the camera) -> -z
    */
    constexpr glm::vec3 bodyColor = glm::vec3(0.1f);
    constexpr glm::vec3 bumperColor = glm::vec3(0.2f);
    constexpr glm::vec3 windowColor = glm::vec3(0.6f, 0.7f, 0.8f);
    constexpr uint8_t alpha = 128;  // Half transparent
    constexpr glm::vec3 blinkerColor = glm::vec3(1.0f, 0.5f, 0.0f);


    /* --- START MAIN BODY ---*/
    {
        CreateBody(shader, bodyColor);
    }
    /* --- STOP MAIN BODY ---*/

    /* --- START SEATS ---*/
    {
        constexpr glm::vec3 seatColor = glm::vec3(0.82, 0.71, 0.55);
        // Front seats
        // Left seat
        CreateSeat(shader,
            seatColor,
            glm::vec3(-0.5f, 0.6f, 0.1f),
            glm::vec3(0.6f, 0.7f, 0.6f)
        );
        // Right seat
        CreateSeat(shader,
            seatColor,
            glm::vec3(0.5f, 0.6f, 0.1f),
            glm::vec3(0.6f, 0.7f, 0.6f)
        );

        // Backseat
        CreateSeat(shader,
            seatColor,
            glm::vec3(0.0f, 0.6f, -0.8f),
            glm::vec3(1.8f, 0.7f, 0.7f)
        );
    }
    /* --- STOP SEATS ---*/

    /* --- START STEERING WHEEL ---*/
    {
        constexpr glm::vec3 steeringWheelColor = glm::vec3(0.3f);
        CreateSteeringWheel(shader,
            steeringWheelColor
        );
    }
    /* --- STOP STEERING WHEEL ---*/

    /* --- START HANDLES --- */
    {
        CreateHandles(shader, bumperColor);
    }
    /* --- STOP HANDLES --- */

    /* --- START BUMPERS ---*/
    {
        CreateBumpers(shader, bumperColor);
    }
    /* --- STOP BUMPERS ---*/

    /* --- START LICENSE PLATES --- */
    {
        constexpr glm::vec3 plateColor = glm::vec3(0.85f);
        CreatePlates(shader, plateColor);
    }
    /* --- STOP LICENSE PLATES --- */

    /* --- START FENDERS ---*/
    {
        constexpr float fX_pos = 1.15f;
        constexpr float fY_pos = 0.83f;
        constexpr float fZ_front_hub = 1.5f;
        constexpr float fZ_rear_hub = -1.5f;

        constexpr glm::vec3 topScale(0.12f, 0.04f, 0.5f);
        constexpr glm::vec3 sideScale(0.12f, 0.04f, 0.35f);
        constexpr float slantAngle = glm::radians(45.0f);

        // Helper to make a fender
        auto AddTrapezoidFender = [&](float x, float y, float z, bool isLeft) {
            // Flat top
            AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
                bodyColor,
                glm::vec3(x, y + 0.02f, z),
                topScale
            );

            // Front slant (towards the front of the car)
            AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
                bodyColor,
                glm::vec3(x, y - 0.092f, z + 0.345f),
                sideScale,
                glm::vec3(slantAngle, 0, 0)
            );

            // Rear slant (towards the rear of the car)
            AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
                bodyColor,
                glm::vec3(x, y - 0.092f, z - 0.345f),
                sideScale,
                glm::vec3(-slantAngle, 0, 0)
            );
        };

        // Front Left
        AddTrapezoidFender(-fX_pos, fY_pos, fZ_front_hub, true);
        // Front Right
        AddTrapezoidFender(fX_pos, fY_pos, fZ_front_hub, false);
        // Rear Left
        AddTrapezoidFender(-fX_pos, fY_pos, fZ_rear_hub, true);
        // Rear Right
        AddTrapezoidFender(fX_pos, fY_pos, fZ_rear_hub, false);
    }
    /* --- STOP FENDERS ---*/

    /* --- START CABIN MAIN ---*/
    {
        CreateCabinMain(shader, alpha, bodyColor, windowColor);
    }
    /* --- STOP CABIN MAIN ---*/

    /* --- START SIDE MIRRORS --- */
    {
        CreateMirrors(shader, bodyColor, windowColor);
    }
    /* --- STOP SIDE MIRRORS --- */

    /* --- START GRILLE --- */
    {
        constexpr glm::vec3 grilleBgColor = glm::vec3(0.02f);
        constexpr glm::vec3 grilleBarColor = glm::vec3(0.25f);
        CreateGrille(shader, grilleBgColor, grilleBarColor);
    }
    /* --- STOP GRILLE --- */

    /* --- START CABIN SIDE --- */
    {
        CreateCabinSide(shader, FacePosition::Left, alpha, bodyColor, windowColor);
        CreateCabinSide(shader, FacePosition::Right, alpha, bodyColor, windowColor);
    }
    /* --- STOP CABIN SIDE --- */

    /* --- START LIGHTBAR --- */
    {
        constexpr glm::vec3 lightBarBoxColor = glm::vec3(0.2f);
        CreateLightBar(shader, lightBarBoxColor);
    }
    /* --- STOP LIGHTBAR --- */

    /* --- START WHEELS --- */
    {
        constexpr glm::vec3 wheelColor = glm::vec3(0.1f);
        CreateWheels(shader, wheelColor);
    }
    /* --- STOP WHEELS --- */

    /* --- START FRONT LIGHTS --- */
    {
        constexpr glm::vec3 shortBeamColor = glm::vec3(1.0f);
        constexpr glm::vec3 highBeamColor = glm::vec3(1.0f, 1.0f, 0.8f);
        CreateFrontLights(shader, shortBeamColor, highBeamColor, blinkerColor);
    }
    /* --- STOP FRONT LIGHTS --- */

   /* --- START BACK LIGHTS --- */
    {
        constexpr glm::vec3 brakeLightColor = glm::vec3(1.0f, 0.0f, 0.0f);
        constexpr glm::vec3 reverseLightColor = glm::vec3(0.96f, 0.96f, 0.72f);
        CreateBackLights(shader, brakeLightColor, reverseLightColor, blinkerColor);
    }
    /* --- STOP BACK LIGHTS --- */
}

void PoliceCarModel::CreateBody(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float length = 4.8f;
    constexpr float width = 2.2f;
    constexpr float height = 0.6f;
    constexpr float thickness = 0.2f;

    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
        color,
        glm::vec3(0, 0.55f, 0), 
        glm::vec3(width, thickness, length)
    );

    float bodyCenterY = 0.75f;

    // Left wall
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
        color,
        glm::vec3(-(width / 2) + (thickness / 2), bodyCenterY, 0),
        glm::vec3(thickness, height, length)
    );

    // Right wall
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
        color,
        glm::vec3((width / 2) - (thickness / 2), bodyCenterY, 0),
        glm::vec3(thickness, height, length)
    );

    // Front (hood)
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
        color,
        glm::vec3(0, bodyCenterY, (length / 2) - 0.87f),
        glm::vec3(width, height, 1.75f)
    );

    // Rear (trunk)
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
        color,
        glm::vec3(0, bodyCenterY, -(length / 2) + 0.65f),
        glm::vec3(width, height, 1.3f)
    );
}

void PoliceCarModel::CreateCabinMain(const std::weak_ptr<Shader>& shader, uint8_t alpha, const glm::vec3& bodyColor, const glm::vec3& windowColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float windShieldAngle = 45.0f;
    constexpr float rearWindowAngle = 35.0f;

    // Windshield
    AddWindowPart(shader, WindowType::Flat, PoliceCarPartIndex::None,
        alpha,
        windowColor,
        glm::vec3(0, 1.13f + 0.01f, 0.67f),
        glm::vec3(2.0f, 0.02f, 0.65f),
        glm::vec3(glm::radians(windShieldAngle), 0, 0)
    );

    // Rear Window
    AddWindowPart(shader, WindowType::Flat, PoliceCarPartIndex::None,
        alpha,
        windowColor,
        glm::vec3(0, 1.15f, -1.45f),
        glm::vec3(2.00f, 0.02f, 0.75f),
        glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
    );

    // Roof
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(0, 1.357f, -0.35f),
        glm::vec3(2.05f, 0.05f, 1.6f)
    );

    float pillarY = 1.15f + 0.025f;

    // Left front pillar
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(-0.985f, pillarY - 0.005f, 0.625f),
        glm::vec3(0.08f, 0.05f, 0.553f),
        glm::vec3(glm::radians(windShieldAngle), 0, 0)
    );

    // Right front pillar
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(0.985f, pillarY - 0.005f, 0.625f),
        glm::vec3(0.08f, 0.05f, 0.553f),
        glm::vec3(glm::radians(windShieldAngle), 0, 0)
    );

    // Right rear pillar
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(-0.95f, pillarY, -1.4f),
        glm::vec3(0.15f, 0.05f, 0.65f),
        glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
    );

    // Left rear pillar
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(0.95f, pillarY, -1.4f),
        glm::vec3(0.15f, 0.05f, 0.65f),
        glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
    );
}

void PoliceCarModel::CreateSeat(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    // base
    glm::vec3 baseScale = glm::vec3(scale.x, scale.y * 0.25f, scale.z);

    glm::vec3 basePos = pos;
    basePos.y += (baseScale.y * 0.5f);

    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
        color,
        basePos,
        baseScale
    );

    // Backrest
    glm::vec3 backrestScale = glm::vec3(scale.x, scale.y * 0.75f, scale.z * 0.3f);

    glm::vec3 backrestPos = pos;
    // Move up and back
    backrestPos.y += (scale.y * 0.1f) + (backrestScale.y * 0.5f);
    backrestPos.z -= (scale.z * 0.35f) + 0.07f;

    glm::vec3 backrestRot = glm::vec3(glm::radians(-15.0f), 0.0f, 0.0f);

    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
        color,
        backrestPos,
        backrestScale,
        backrestRot
    );
}

void PoliceCarModel::CreateWheels(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float wX = 1.05f;
    constexpr float wY = 0.45f;
    constexpr float wZ = 1.5f;
    // THESE ARE THE 'PRE ROTATED' SCALES!!! AFTER ROTATION Y BECOMES X
    constexpr glm::vec3 wheelScale = glm::vec3(0.7f, 0.4f, 0.7f);

    // Looking at the car from the front.
    // Front left wheel
    AddWheel(shader, PoliceCarPartIndex::None,
        color,
        glm::vec3(-wX, wY, wZ),
        wheelScale
    );

    // Front right wheel
    AddWheel(shader, PoliceCarPartIndex::None,
        color,
        glm::vec3(wX, wY, wZ),
        wheelScale
    );

    // Back left wheel
    AddWheel(shader, PoliceCarPartIndex::None,
        color,
        glm::vec3(-wX, wY, -wZ),
        wheelScale
    );

    // Back right wheel.
    AddWheel(shader, PoliceCarPartIndex::None,
        color,
        glm::vec3(wX, wY, -wZ),
        wheelScale
    );

}

void PoliceCarModel::CreateHandles(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float tabWidth = 0.3f;
    constexpr float tabHeight = 0.1f;
    constexpr float tabDepth = 0.01f;
    constexpr float tabZ = -2.4f;
    constexpr float tabY = 0.8f + (tabHeight * 0.5f);

    // Trunk handle
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(0.0f, tabY, tabZ),
        glm::vec3(tabWidth, tabHeight, tabDepth)
    );

    constexpr float hX = 1.11f;
    constexpr float hY = 0.925f;

    constexpr float hZ_Front = -0.05f;
    constexpr float hZ_Rear = -0.9f;
    constexpr glm::vec3 handleScale(0.02f, 0.05f, 0.25f);

    // Looking at the front of the car!
    // Left Front
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(-hX, hY, hZ_Front),
        handleScale
    );
    // Right Front
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(hX, hY, hZ_Front),
        handleScale
    );
    // Left Rear
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(-hX, hY, hZ_Rear),
        handleScale
    );
    // Right Rear
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(hX, hY, hZ_Rear),
        handleScale
    );
}

void PoliceCarModel::CreateBumpers(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float bWidth = 2.3f;
    constexpr float bHeight = 0.25f;
    constexpr float bThick = 0.12f;
    constexpr float bY = 0.475f;

    constexpr float bZ_Front = 2.45f;
    constexpr float bZ_Rear = -2.45f;

    constexpr float sideLen = 0.4f;
    constexpr glm::vec3 sideBumperScale(0.08f, bHeight, sideLen);
    constexpr float sideX = (bWidth / 2.0f) - 0.040f;

    // Front bumper
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(0, bY, bZ_Front),
        glm::vec3(bWidth, bHeight, bThick)
    );

    // Left Wrap
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(-sideX, bY, bZ_Front - (sideLen / 2.0f)),
        sideBumperScale
    );

    // Right Wrap
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(sideX, bY, bZ_Front - (sideLen / 2.0f)),
        sideBumperScale
    );

    // Rear bumper
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(0, bY, bZ_Rear),
        glm::vec3(bWidth, bHeight, bThick)
    );

    // Left Wrap
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(-sideX, bY, bZ_Rear + (sideLen / 2.0f)),
        sideBumperScale
    );

    // Right Wrap
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(sideX, bY, bZ_Rear + (sideLen / 2.0f)),
        sideBumperScale
    );

    // Side skirts
    // Original Y was 0.35f. Height is 0.15f.
    // New center = 0.35f + (0.15f / 2) = 0.425f
    float skirtY = 0.425f;

    // Side skirt, Right
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(1.1f, skirtY, 0.0f),
        glm::vec3(0.05f, 0.15f, 2.1f)
    );

    // Side skirt, Left
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(-1.1f, skirtY, 0.0f),
        glm::vec3(0.05f, 0.15f, 2.1f)
    );
}

void PoliceCarModel::CreatePlates(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float bZ_Front = 2.45f;
    constexpr float bZ_Rear = -2.45f;
    constexpr glm::vec3 plateSize(0.55f, 0.15f, 0.01f);
    constexpr float plateY = 0.475f;

    // Front Plate
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(0.0f, plateY, bZ_Front + 0.06f),
        plateSize
    );

    // Rear Plate 
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        color,
        glm::vec3(0.0f, plateY, bZ_Rear - 0.06f),
        plateSize
    );
}

void PoliceCarModel::CreateMirrors(const std::weak_ptr<Shader>& shader, const glm::vec3& bodyColor, const glm::vec3& mirrorColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float mX_Neck = 1.00f;
    constexpr float mX_Housing = 1.23f;
    constexpr float mX_Mirror = mX_Housing;

    constexpr float mY_Neck = 1.05f + 0.025f;          // 1.075f
    constexpr float mY_Housing = 1.02f + 0.0525f;      // 1.0725f
    constexpr float mY_Mirror = (1.02f + 0.01f) + 0.045f; // 1.075f

    constexpr float mZ = 0.70f;

    constexpr glm::vec3 neckScale(0.2f, 0.05f, 0.05f);
    constexpr glm::vec3 houseScale(0.21f, 0.105f, 0.10f);
    constexpr glm::vec3 glassScale(0.16f, 0.09f, 0.01f);

    constexpr float leftAngle = glm::radians(-15.0f);
    constexpr float rightAngle = glm::radians(15.0f);

    // Left side (looking at the front of the car)
    // Neck
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(-(mX_Neck + 0.075f), mY_Neck, mZ),
        neckScale
    );

    // Housing
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(-mX_Housing, mY_Housing, mZ),
        houseScale,
        glm::vec3(0, leftAngle, 0)
    );

    // Glass
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        mirrorColor,
        glm::vec3(-mX_Mirror + 0.01f, mY_Mirror, mZ - 0.05f),
        glassScale,
        glm::vec3(0, leftAngle, 0)
    );

    // Right side (looking at the front of the car)
    // Neck
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(mX_Neck + 0.075f, mY_Neck, mZ),
        neckScale
    );

    // Housing
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(mX_Housing, mY_Housing, mZ),
        houseScale,
        glm::vec3(0, rightAngle, 0)
    );

    // Glass
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        mirrorColor,
        glm::vec3(mX_Mirror - 0.01f, mY_Mirror, mZ - 0.05f),
        glassScale,
        glm::vec3(0, rightAngle, 0)
    );
}

void PoliceCarModel::CreateGrille(const std::weak_ptr<Shader>& shader, const glm::vec3& grilleBgColor, const glm::vec3& grilleColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float gY = 0.64f;
    constexpr float gZ = 2.41f;
    constexpr float gWidth = 0.845f;
    constexpr float gHeight = 0.32f;

    // Void
    float grilleBgY = gY + (gHeight * 0.5f); // 0.64 + 0.16 = 0.80f

    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        grilleBgColor,
        glm::vec3(0.0f, grilleBgY, gZ),
        glm::vec3(gWidth, gHeight, 0.02f)
    );

    // Vertical bars
    float barHeight = gHeight + 0.01f;
    float barY = gY + (barHeight * 0.5f); // 0.64 + 0.165 = 0.805f

    for (int i = 0; i < 9; i++)
    {
        float xOffset = (i - 1) * 0.1f;
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            grilleColor,
            glm::vec3(-0.3f + xOffset, barY, gZ + 0.01f),
            glm::vec3(0.05f, barHeight, 0.04f)
        );
    }
}

void PoliceCarModel::CreateCabinSide(const std::weak_ptr<Shader>& shader, FacePosition side, uint8_t alpha, const glm::vec3& bodyColor, const glm::vec3& windowColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    float multplier = HasFlag(side, FacePosition::Left) ? -1.0f : 1.0f;
    // Side window
    AddWindowPart(shader, WindowType::Flat, PoliceCarPartIndex::None,
        alpha,
        windowColor,
        glm::vec3(0.99f * multplier, 1.195f, -0.34f),
        glm::vec3(0.02f, 0.35f, 1.5f)
    );

    // Triangle windows
    // Rear Triangle
    AddWindowPart(shader, WindowType::Triangle, PoliceCarPartIndex::None,
        alpha,
        windowColor,
        glm::vec3(0.99f * multplier, 1.20f, -1.36f),
        glm::vec3(0.02f, 0.35f, 0.5f),
        glm::vec3(0.0f, glm::radians(180.0f), 0.0f)
    );

    // Front Triangle
    AddWindowPart(shader, WindowType::Triangle, PoliceCarPartIndex::None,
        alpha,
        windowColor,
        glm::vec3(0.99f * multplier, 1.20f, 0.60f),
        glm::vec3(0.02f, 0.35f, 0.35f)
    );

    // Side pillars
    float pillarY = 1.075f;
    glm::vec3 pillarScale(0.08f, 0.05f, 0.61f);
    glm::vec3 pillarRot(glm::radians(90.0f), 0, 0);

    // Rear Side Pillar
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(0.97f * multplier, pillarY, -1.10f),
        pillarScale,
        pillarRot
    );

    // Front Side Pillar
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(0.97f * multplier, pillarY, 0.4f),
        pillarScale,
        pillarRot
    );

    // Center Side Pillar
    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        bodyColor,
        glm::vec3(0.97f * multplier, pillarY, -0.3f),
        pillarScale,
        pillarRot
    );
}

void PoliceCarModel::CreateLightBar(const std::weak_ptr<Shader>& shader, const glm::vec3& boxColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float lbY = 1.35f;
    constexpr float lbZ = -0.2f;
    constexpr float lbX = 0.55f;

    // Central box
    float sirenY = lbY + 0.09f; // 1.49f

    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
        boxColor,
        glm::vec3(0, sirenY, lbZ),
        glm::vec3(0.4f, 0.18f, 0.3f)
    );

    float lightY = lbY + 0.075f; // 1.475f

    // Left light bar (red)
    AddPointLightPart(shader, PoliceCarLightType::LightBar, FacePosition::Left, PoliceCarPartIndex::LightBar_L, ShadowCubeIndex::Lightbar_L,
        glm::vec3(1, 0, 0),
        glm::vec3(-lbX, lightY, lbZ),
        glm::vec3(0.8f, 0.15f, 0.25f)
    );

    // Right light bar (blue)
    AddPointLightPart(shader, PoliceCarLightType::LightBar, FacePosition::Right, PoliceCarPartIndex::LightBar_R, ShadowCubeIndex::Lightbar_R,
        glm::vec3(0, 0, 1),
        glm::vec3(lbX, lightY, lbZ),
        glm::vec3(0.8f, 0.15f, 0.25f)
    );
}

void PoliceCarModel::CreateFrontLights(const std::weak_ptr<Shader>& shader, const glm::vec3& shortBeamColor, const glm::vec3& highBeamColor, const glm::vec3& blinkerColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    // Left Front Light Cluster
    // LOOKING AT THE CAR'S FRONT
    constexpr float clusterX = -0.75f;
    constexpr float clusterY = 0.8f;
    constexpr float clusterZ = 2.38f;

    constexpr float shift = 0.05f;

    // Upper Left short beam
    AddSpotLightPart(shader, PoliceCarLightType::Headlight, FacePosition::Left, PoliceCarPartIndex::HeadLight_L, ShadowMapIndex::PL_Headlight_L,
        shortBeamColor,
        glm::vec3(clusterX, clusterY + 0.1f + shift, clusterZ),
        glm::vec3(0.5f, 0.1f, 0.1f)
    );

    // Bottom left high beam
    AddSpotLightPart(shader, PoliceCarLightType::HighBeam, FacePosition::Left, PoliceCarPartIndex::HighBeam_L, ShadowMapIndex::PL_Highbeam_L,
        highBeamColor,
        glm::vec3(clusterX + 0.125f, clusterY + shift, clusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Bottom left blinker
    AddPointLightPart(shader, PoliceCarLightType::Blinker, FacePosition::Left | FacePosition::Front, PoliceCarPartIndex::Blinker_FL, ShadowCubeIndex::Blinker_FL,
        blinkerColor,
        glm::vec3(clusterX - 0.125f, clusterY + shift, clusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Right Front Light Cluster
    // Upper right short beam
    AddSpotLightPart(shader, PoliceCarLightType::Headlight, FacePosition::Right, PoliceCarPartIndex::HeadLight_R, ShadowMapIndex::PL_Headlight_R,
        shortBeamColor,
        glm::vec3(-clusterX, clusterY + 0.1f + shift, clusterZ),
        glm::vec3(0.5f, 0.1f, 0.1f)
    );

    // Bottom right high beam
    AddSpotLightPart(shader, PoliceCarLightType::HighBeam, FacePosition::Right, PoliceCarPartIndex::HighBeam_R, ShadowMapIndex::PL_Highbeam_R,
        highBeamColor,
        glm::vec3(-clusterX - 0.125f, clusterY + shift, clusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Bottom right blinker
    AddPointLightPart(shader, PoliceCarLightType::Blinker, FacePosition::Right | FacePosition::Front, PoliceCarPartIndex::Blinker_FR, ShadowCubeIndex::Blinker_FR,
        blinkerColor,
        glm::vec3(-clusterX + 0.125f, clusterY + shift, clusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );
}

void PoliceCarModel::CreateBackLights(const std::weak_ptr<Shader>& shader, const glm::vec3& brakeLightColor, const glm::vec3& reverLightColor, const glm::vec3& blinkerColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float rClusterX = -0.75f;
    constexpr float rClusterY = 0.8f;
    constexpr float rClusterZ = -2.38f;

    constexpr float rShift = 0.05f;

    // LEFT SIDE CLUSTER (LOOKING AT THE FRONT OF CAR!!!)
    // Top left brake light
    AddSpotLightPart(shader, PoliceCarLightType::BrakeLight, FacePosition::Left, PoliceCarPartIndex::RearLight_L, ShadowMapIndex::PL_Rearlight_L,
        brakeLightColor,
        glm::vec3(rClusterX, rClusterY + 0.1f + rShift, rClusterZ),
        glm::vec3(0.5f, 0.1f, 0.1f)
    );

    // Bottom left reverse light
    AddSpotLightPart(shader, PoliceCarLightType::ReverseLight, FacePosition::Left, PoliceCarPartIndex::ReverseLight_L, ShadowMapIndex::PL_Reverselight_L,
        reverLightColor,
        glm::vec3(rClusterX + 0.125f, rClusterY + rShift, rClusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Bottom left blinker light
    AddPointLightPart(shader, PoliceCarLightType::Blinker, FacePosition::Left | FacePosition::Behind, PoliceCarPartIndex::Blinker_BL, ShadowCubeIndex::Blinker_BL,
        blinkerColor,
        glm::vec3(rClusterX - 0.125f, rClusterY + rShift, rClusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // RIGHT SIDE CLUSTER
    // Top right brake light
    AddSpotLightPart(shader, PoliceCarLightType::BrakeLight, FacePosition::Right, PoliceCarPartIndex::RearLight_R, ShadowMapIndex::PL_Rearlight_R,
        brakeLightColor,
        glm::vec3(-rClusterX, rClusterY + 0.1f + rShift, rClusterZ),
        glm::vec3(0.5f, 0.1f, 0.1f)
    );

    // Bottom right reverse light
    AddSpotLightPart(shader, PoliceCarLightType::ReverseLight, FacePosition::Right, PoliceCarPartIndex::ReverseLight_R, ShadowMapIndex::PL_Reverselight_R,
        reverLightColor,
        glm::vec3(-rClusterX - 0.125f, rClusterY + rShift, rClusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Bottom right blinker
    AddPointLightPart(shader, PoliceCarLightType::Blinker, FacePosition::Right | FacePosition::Behind, PoliceCarPartIndex::Blinker_BR, ShadowCubeIndex::Blinker_BR,
        blinkerColor,
        glm::vec3(-rClusterX + 0.125f, rClusterY + rShift, rClusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );
}

void PoliceCarModel::CreateSteeringWheel(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float RHD_X = 0.55f;
    constexpr float DASH_Z = 0.5f;
    constexpr float COLUMN_HEIGHT = 0.85f;
    constexpr float TILT_ANGLE = glm::radians(25.0f);

    // Steering wheel column
    constexpr glm::vec3 steeringRot = glm::vec3(TILT_ANGLE, 0.0f, 0.0f);
    constexpr glm::vec3 columnBasePos = glm::vec3(RHD_X, COLUMN_HEIGHT, DASH_Z);

    glm::vec3 columnPos = columnBasePos + glm::vec3(0.0f, 0.04f, 0.2f);
    constexpr glm::vec3 columnScale = glm::vec3(0.08f, 0.08f, 0.4f);

    AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
        color * 0.6f, columnPos, columnScale, steeringRot
    );

    // Steering wheel
    constexpr float RIM_VISUAL_HEIGHT = 0.97f;
    constexpr glm::vec3 rimPos = glm::vec3(RHD_X, RIM_VISUAL_HEIGHT, DASH_Z);

    glm::vec3 rimRot = glm::vec3(TILT_ANGLE + glm::radians(90.0f), 0.0f, 0.0f);

    float rimRadius = 0.15f;
    float rimHeight = 0.04f;

    auto mesh = std::make_unique<CylinderMesh>(
        shader,
        rimRadius,
        rimHeight,
        32,
        Transform{
            .position = rimPos,
            .rotation = rimRot,
            .scale = glm::vec3(1.0f)
        },
        color
    );
    CreatePart(std::move(mesh), PoliceCarMeshType::Steering, PoliceCarPartIndex::None);
}

std::shared_ptr<PoliceCarModel> PoliceCarModel::CreateInstance(const Transform& t){ return std::make_shared<PoliceCarModel>(data, t); }

void PoliceCarModel::AddBodyPart(const std::weak_ptr<Shader>& shader, PoliceCarMeshType type, PoliceCarPartIndex index, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);
    CreatePart(std::move(mesh), type, index, pos);
}

void PoliceCarModel::AddSpotLightPart(const std::weak_ptr<Shader>& shader, PoliceCarLightType lType, FacePosition side, PoliceCarPartIndex index, ShadowMapIndex shadowMapindex, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);
    CreatePart(std::move(mesh), PoliceCarMeshType::Light, index, pos);
}

void PoliceCarModel::AddPointLightPart(const std::weak_ptr<Shader>& shader, PoliceCarLightType lType, FacePosition side, PoliceCarPartIndex index, ShadowCubeIndex shadowCubeIndex, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);
    CreatePart(std::move(mesh), PoliceCarMeshType::Light, index, pos);
}

void PoliceCarModel::AddWheel(const std::weak_ptr<Shader>& shader, PoliceCarPartIndex index, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    Transform t{ .position = pos, .rotation = glm::vec3(0, 0, glm::radians(90.0f)) + rot, .scale = scale };
    auto mesh = std::make_unique<CylinderMesh>(shader, 0.5f, 1.0f, 32, t, color);

    CreatePart(std::move(mesh), PoliceCarMeshType::Wheel, index, pos);
}

void PoliceCarModel::AddWindowPart(const std::weak_ptr<Shader>& shader, WindowType type, PoliceCarPartIndex index, uint8_t alpha, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    if (!data->windowTex)
    {
        data->windowTex = Texture::CreateSolidColor(
            static_cast<unsigned char>(color.r * 255),
            static_cast<unsigned char>(color.g * 255),
            static_cast<unsigned char>(color.b * 255),
            alpha,
            "texture_diffuse"
        );
    }

    std::unique_ptr<Mesh> mesh = nullptr;

    switch (type)
    {
        case WindowType::Triangle:
        {
            mesh = std::make_unique<WedgeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);
            break;
        }
        case WindowType::Flat:
        default:
        {
            mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);
            break;
        }
    }

    PoliceCarPart& part = CreatePart(std::move(mesh), PoliceCarMeshType::Window, index, pos);
    part.meshData->isTransparent = true;
}

void PoliceCarModel::Bake(const std::weak_ptr<Shader>& shader)
{
    data->bakedBodyMesh.reset();
    data->bakedAlphaMesh.reset();

    data->bakedBodyMesh = PerformBakePass(shader, BakePassType::Solid);

    data->bakedAlphaMesh = PerformBakePass(shader, BakePassType::Alpha);

    if (data->bakedAlphaMesh && data->windowTex) 
    {
        data->bakedAlphaMesh->SetTexture(data->windowTex);
    }
}

std::unique_ptr<Mesh> PoliceCarModel::PerformBakePass(const std::weak_ptr<Shader>& shader, BakePassType passType)
{
    std::vector<float> allVertices;
    std::vector<unsigned int> allIndices;

    std::vector<MeshCache> caches;
    size_t totalVertFloats = 0;
    size_t totalIndices = 0;

    for (auto& part : data->parts)
    {
        if (!part.meshData)
        {
            continue;
        }

        bool isWindow = (part.meshData->type == PoliceCarMeshType::Window);

        if (passType == BakePassType::Alpha && !isWindow) continue;
        if (passType == BakePassType::Solid && isWindow) continue;

        auto& mesh = part.meshData->mesh;
        MeshCache cache;
        cache.verts = mesh->GetVertexData();
        cache.inds = mesh->GetIndexData();
        cache.transform = mesh->GetTransform().GetModelMatrix();

        totalVertFloats += cache.verts.size();
        totalIndices += cache.inds.size();
        caches.push_back(std::move(cache));
    }

    if (caches.empty()) return nullptr;

    allVertices.reserve(totalVertFloats);
    allIndices.reserve(totalIndices);

    for (const auto& cache : caches)
    {
        const unsigned int vertOffset = static_cast<unsigned int>(allVertices.size() / STRIDE);
        glm::mat3 normMat = glm::mat3(glm::transpose(glm::inverse(cache.transform)));

        for (size_t v = 0; v < cache.verts.size(); v += STRIDE)
        {
            glm::vec4 pos = cache.transform * glm::vec4(cache.verts[v], cache.verts[v + 1], cache.verts[v + 2], 1.0f);
            allVertices.push_back(pos.x);
            allVertices.push_back(pos.y);
            allVertices.push_back(pos.z);

            glm::vec3 norm = glm::normalize(normMat * glm::vec3(cache.verts[v + 3], cache.verts[v + 4], cache.verts[v + 5]));
            allVertices.push_back(norm.x);
            allVertices.push_back(norm.y);
            allVertices.push_back(norm.z);

            // Copy remaining data (UVs, Tangents, Colors)
            size_t startOfRemainingData = v + 6;
            allVertices.insert(allVertices.end(),
                cache.verts.begin() + startOfRemainingData,
                cache.verts.begin() + startOfRemainingData + 8
            );
        }

        for (auto idx : cache.inds) {
            allIndices.push_back(idx + vertOffset);
        }
    }

    if (allVertices.empty())
    {
        return nullptr;
    }

    return std::make_unique<Mesh>(
        allVertices.data(), 
        allVertices.size(),
        shader,
        Transform{}, 
        glm::vec3(1.0f),
        allIndices.data(), 
        allIndices.size()
    );
}

void PoliceCarModel::OnDraw(std::shared_ptr<Shader> customShader) const {}