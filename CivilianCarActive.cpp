#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <queue>

#include "CivilianCarActive.h"

#include "MeshCache.h"
#include "WedgeMesh.h"
#include "CubeMesh.h"
#include "CylinderMesh.h"
#include "CivilianCarLight.h"
#include "CityModel.h"

CivilianCarActive::CivilianCarActive(const std::weak_ptr<Shader>& shader, std::vector<std::vector<CityTileType>> cityGrid, const Transform& t) :
    transform(t),
    cityGrid(cityGrid)
{
    data = std::make_shared<CivilianCarMeshData>();
    InitParts(shader);
    Bake(shader);
}

void CivilianCarActive::Update(double deltaTime)
{
    float dt = static_cast<float>(deltaTime);
    pathUpdateTimer += dt;

    // If we are out of nodes OR if enough time passed to re-evaluate the escape
    if (path.empty() || currentPathIndex >= path.size() || pathUpdateTimer > 2.0f)
    {
        glm::ivec2 start = WorldToGrid(transform.position);
        glm::ivec2 pGrid = WorldToGrid(playerPos);

        glm::ivec2 target = FindFleeTarget(pGrid, cityGrid);
        path = CalculatePath(start, target, cityGrid);

        // Skip the first node if we are already basically there
        currentPathIndex = (path.size() > 1) ? 1 : 0;
        pathUpdateTimer = 0.0f;
    }

    if (!path.empty() && currentPathIndex < path.size())
    {
        int lookAheadIndex = currentPathIndex;
        if (speed > MAX_SPEED * 0.6f && currentPathIndex + 1 < path.size()) 
        {
            lookAheadIndex = currentPathIndex + 1;
        }

        glm::vec3 targetWorld = GridToWorld(path[lookAheadIndex]);
        glm::vec3 toTarget = targetWorld - transform.position;
        float distToNode = glm::length(toTarget);

        float desiredYaw = glm::degrees(atan2(toTarget.x, toTarget.z));
        float angleDiff = desiredYaw - currentAngle;

        while (angleDiff > 180.0f) angleDiff -= 360.0f;
        while (angleDiff < -180.0f) angleDiff += 360.0f;

        steerAngle = glm::clamp(angleDiff, -TURN_ANGLE, TURN_ANGLE);

        // Only brake if the turn is TRULY sharp (> 40 degrees)
        if (glm::abs(angleDiff) > 40.0f && speed > MAX_SPEED * 0.4f)
        {
            isBraking = true;
            speed -= DECELERATION * 1.5f * dt;
        }
        else
        {
            isBraking = false;
            speed += ACCELERATION * dt;
        }

        if (distToNode < 7.0f) currentPathIndex++;
    }

    UpdatePhysics(dt);

    for (auto& part : parts)
    {
        if (part.lightData && part.lightData->spotLight)
        {
            HandleSpotLight(part);
        }

        if (part.meshData)
        {
            switch (part.meshData->type)
            {
                case CivilianCarMeshType::Wheel:
                    UpdateWheelTransform(part);
                    break;
                case CivilianCarMeshType::Steering:
                    UpdateSteeringTransform(part);
                    break;
                default:
                    break;
            }
        }
    }
}

void CivilianCarActive::Draw(std::shared_ptr<Shader> customShader) const
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

    for (size_t idx : dynamicPartIndices)
    {
        auto& part = parts[idx];
        if (!part.meshData)
        {
            continue;
        }

        auto& mesh = part.meshData->mesh;

        activeShader->UseShader();

        bool isLight = (part.meshData->type == CivilianCarMeshType::Light);
        glUniform1i(activeShader->GetUniformLocation("isLightMesh"), isLight);

        if (isLight)
        {
            glUniform1f(activeShader->GetUniformLocation("lightMeshIntensity"), part.meshData->lightMeshIntensity);
        }

        glm::mat4 partLocalMatrix;
        if (part.meshData->type == CivilianCarMeshType::Wheel)
        {
            partLocalMatrix = CalculateWheelMatrix(part);
        }
        else
        {
            partLocalMatrix = mesh->GetTransform().GetModelMatrix();
        }

        glm::mat4 finalModelMatrix = carMatrix * partLocalMatrix;

        glUniformMatrix4fv(activeShader->GetUniformLocation("modelMatrix"), 1, GL_FALSE, &finalModelMatrix[0][0]);
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(finalModelMatrix)));
        glUniformMatrix3fv(activeShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

        mesh->DrawRaw(customShader);

        if (isLight)
        {
            glUniform1i(activeShader->GetUniformLocation("isLightMesh"), 0);
        }
    }
}

std::vector<CivilianCarLight*> CivilianCarActive::GetAllLights()
{
    std::vector<CivilianCarLight*> lights;
    lights.reserve(parts.size());
    for (auto& part : parts)
    {
        if (part.lightData)
        {
            lights.push_back(part.lightData.get());
        }
    }
    return lights;
}

glm::vec3 CivilianCarActive::GetForward() const
{
    return glm::vec3(sin(glm::radians(currentAngle)), 0.0f, cos(glm::radians(currentAngle)));
}

glm::vec3 CivilianCarActive::GetPosition() const { return transform.position; }

glm::vec3 CivilianCarActive::GetWorldPosOfLight(CivilianCarPartIndex partIndex) const
{
    const CivilianCarPart* part = GetPart(partIndex);
    if (!part)
    {
        return transform.position;
    }

    glm::vec3 localOffset = (part->lightData) ? part->lightData->offset : part->meshData->offset;

    return TransformOffsetToWorld(localOffset);
}

float CivilianCarActive::GetSpotlightOuterAngle(CivilianCarPartIndex partIndex) const
{
    if (auto* part = this->GetPart(partIndex))
    {
        if (part->lightData && part->lightData->spotLight)
        {
            return part->lightData->spotLight->outerCutOff;
        }
    }
    return 1.0f;
}

void CivilianCarActive::SetPlayerPosition(const glm::vec3& playerPos) { this->playerPos = playerPos; }

CivilianCarPart* CivilianCarActive::GetPart(CivilianCarPartIndex index)
{
    auto it = carPartIndex[static_cast<size_t>(index)];
    if (it == -1 || static_cast<size_t>(it) >= parts.size())
    {
        return nullptr;
    }
    return &parts[it];
}

const CivilianCarPart* CivilianCarActive::GetPart(CivilianCarPartIndex index) const
{
    auto it = carPartIndex[static_cast<size_t>(index)];
    if (it == -1 || static_cast<size_t>(it) >= parts.size())
    {
        return nullptr;
    }
    return &parts[it];
}

glm::vec3 CivilianCarActive::TransformOffsetToWorld(const glm::vec3& localOffset) const
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), transform.rotation.y, glm::vec3(0, 1, 0));
    glm::vec3 worldOffset = glm::vec3(rotation * glm::vec4(localOffset, 1.0f));
    return transform.position + worldOffset;
}

CivilianCarPart& CivilianCarActive::CreatePart(std::unique_ptr<Mesh> mesh, CivilianCarMeshType type, CivilianCarPartIndex index, glm::vec3 offset)
{
    CivilianCarPart part;
    part.meshData = std::make_unique<CivilianCarMesh>();
    part.meshData->mesh = std::move(mesh);
    part.meshData->type = type;
    part.meshData->offset = offset;

    carPartIndex[static_cast<size_t>(index)] = parts.size();
    parts.push_back(std::move(part));
    return parts.back();
}

CivilianCarPart CivilianCarActive::CreateBaseLightPart(const std::weak_ptr<Shader>& shader, CivilianCarLightType lType, FacePosition side, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    CivilianCarPart part;

    // Setup Mesh
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);
    part.meshData = std::make_unique<CivilianCarMesh>();
    part.meshData->mesh = std::move(mesh);
    part.meshData->type = CivilianCarMeshType::Light;

    // Setup Light Metadata
    part.lightData = std::make_unique<CivilianCarLight>();
    part.lightData->type = lType;
    part.lightData->offset = pos;
    part.lightData->position.facePosition = side;

    return part;
}

void CivilianCarActive::InitParts(const std::weak_ptr<Shader>& shader)
{
    /*
        Unless specified in another way, looking at the front of the car:
        Right -> +x
        Left -> -x
        Up / Down -> +/-y
        Forwards (towards the camera) -> +z
        Backwards (away from the camera) -> -z
    */
    constexpr glm::vec3 bodyColor = glm::vec3(1.0, 0.647, 0.0);
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
            AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
                bodyColor,
                glm::vec3(x, y + 0.02f, z),
                topScale
            );

            // Front slant (towards the front of the car)
            AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
                bodyColor,
                glm::vec3(x, y - 0.092f, z + 0.345f),
                sideScale,
                glm::vec3(slantAngle, 0, 0)
            );

            // Rear slant (towards the rear of the car)
            AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
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

void CivilianCarActive::CreateBody(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float length = 4.8f;
    constexpr float width = 2.2f;
    constexpr float height = 0.6f;
    constexpr float thickness = 0.2f;

    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(0, 0.55f, 0),
        glm::vec3(width, thickness, length)
    );

    float bodyCenterY = 0.75f;

    // Left wall
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(-(width / 2) + (thickness / 2), bodyCenterY, 0),
        glm::vec3(thickness, height, length)
    );

    // Right wall
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3((width / 2) - (thickness / 2), bodyCenterY, 0),
        glm::vec3(thickness, height, length)
    );

    // Front (hood)
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(0, bodyCenterY, (length / 2) - 0.87f),
        glm::vec3(width, height, 1.75f)
    );

    // Rear (trunk)
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(0, bodyCenterY, -(length / 2) + 0.65f),
        glm::vec3(width, height, 1.3f)
    );
}

void CivilianCarActive::CreateCabinMain(const std::weak_ptr<Shader>& shader, uint8_t alpha, const glm::vec3& bodyColor, const glm::vec3& windowColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float windShieldAngle = 45.0f;
    constexpr float rearWindowAngle = 35.0f;

    // Windshield
    AddWindowPart(shader, WindowType::Flat,
        alpha,
        windowColor,
        glm::vec3(0, 1.13f + 0.01f, 0.67f),
        glm::vec3(2.0f, 0.02f, 0.65f),
        glm::vec3(glm::radians(windShieldAngle), 0, 0)
    );

    // Rear Window
    AddWindowPart(shader, WindowType::Flat,
        alpha,
        windowColor,
        glm::vec3(0, 1.15f, -1.45f),
        glm::vec3(2.00f, 0.02f, 0.75f),
        glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
    );

    // Roof
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(0, 1.357f, -0.35f),
        glm::vec3(2.05f, 0.05f, 1.6f)
    );

    float pillarY = 1.15f + 0.025f;

    // Left front pillar
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(-0.985f, pillarY - 0.005f, 0.625f),
        glm::vec3(0.08f, 0.05f, 0.553f),
        glm::vec3(glm::radians(windShieldAngle), 0, 0)
    );

    // Right front pillar
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(0.985f, pillarY - 0.005f, 0.625f),
        glm::vec3(0.08f, 0.05f, 0.553f),
        glm::vec3(glm::radians(windShieldAngle), 0, 0)
    );

    // Right rear pillar
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(-0.95f, pillarY, -1.4f),
        glm::vec3(0.15f, 0.05f, 0.65f),
        glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
    );

    // Left rear pillar
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(0.95f, pillarY, -1.4f),
        glm::vec3(0.15f, 0.05f, 0.65f),
        glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
    );
}

void CivilianCarActive::CreateSeat(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    // base
    glm::vec3 baseScale = glm::vec3(scale.x, scale.y * 0.25f, scale.z);

    glm::vec3 basePos = pos;
    basePos.y += (baseScale.y * 0.5f);

    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
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

    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        backrestPos,
        backrestScale,
        backrestRot
    );
}

void CivilianCarActive::CreateWheels(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float wX = 1.05f;
    constexpr float wY = 0.45f;
    constexpr float wZ = 1.5f;
    // THESE ARE THE 'PRE ROTATED' SCALES!!! AFTER ROTATION Y BECOMES X
    constexpr glm::vec3 wheelScale = glm::vec3(0.7f, 0.4f, 0.7f);

    // Looking at the car from the front.
    // Front left wheel
    AddWheel(shader,
        color,
        glm::vec3(-wX, wY, wZ),
        wheelScale
    );

    // Front right wheel
    AddWheel(shader,
        color,
        glm::vec3(wX, wY, wZ),
        wheelScale
    );

    // Back left wheel
    AddWheel(shader,
        color,
        glm::vec3(-wX, wY, -wZ),
        wheelScale
    );

    // Back right wheel.
    AddWheel(shader,
        color,
        glm::vec3(wX, wY, -wZ),
        wheelScale
    );

}

void CivilianCarActive::CreateHandles(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float tabWidth = 0.3f;
    constexpr float tabHeight = 0.1f;
    constexpr float tabDepth = 0.01f;
    constexpr float tabZ = -2.4f;
    constexpr float tabY = 0.8f + (tabHeight * 0.5f);

    // Trunk handle
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
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
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(-hX, hY, hZ_Front),
        handleScale
    );
    // Right Front
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(hX, hY, hZ_Front),
        handleScale
    );
    // Left Rear
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(-hX, hY, hZ_Rear),
        handleScale
    );
    // Right Rear
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(hX, hY, hZ_Rear),
        handleScale
    );
}

void CivilianCarActive::CreateBumpers(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
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
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(0, bY, bZ_Front),
        glm::vec3(bWidth, bHeight, bThick)
    );

    // Left Wrap
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(-sideX, bY, bZ_Front - (sideLen / 2.0f)),
        sideBumperScale
    );

    // Right Wrap
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(sideX, bY, bZ_Front - (sideLen / 2.0f)),
        sideBumperScale
    );

    // Rear bumper
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(0, bY, bZ_Rear),
        glm::vec3(bWidth, bHeight, bThick)
    );

    // Left Wrap
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(-sideX, bY, bZ_Rear + (sideLen / 2.0f)),
        sideBumperScale
    );

    // Right Wrap
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(sideX, bY, bZ_Rear + (sideLen / 2.0f)),
        sideBumperScale
    );

    // Side skirts
    // Original Y was 0.35f. Height is 0.15f.
    // New center = 0.35f + (0.15f / 2) = 0.425f
    float skirtY = 0.425f;

    // Side skirt, Right
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(1.1f, skirtY, 0.0f),
        glm::vec3(0.05f, 0.15f, 2.1f)
    );

    // Side skirt, Left
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(-1.1f, skirtY, 0.0f),
        glm::vec3(0.05f, 0.15f, 2.1f)
    );
}

void CivilianCarActive::CreatePlates(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float bZ_Front = 2.45f;
    constexpr float bZ_Rear = -2.45f;
    constexpr glm::vec3 plateSize(0.55f, 0.15f, 0.01f);
    constexpr float plateY = 0.475f;

    // Front Plate
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(0.0f, plateY, bZ_Front + 0.06f),
        plateSize
    );

    // Rear Plate 
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        color,
        glm::vec3(0.0f, plateY, bZ_Rear - 0.06f),
        plateSize
    );
}

void CivilianCarActive::CreateMirrors(const std::weak_ptr<Shader>& shader, const glm::vec3& bodyColor, const glm::vec3& mirrorColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
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
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(-(mX_Neck + 0.075f), mY_Neck, mZ),
        neckScale
    );

    // Housing
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(-mX_Housing, mY_Housing, mZ),
        houseScale,
        glm::vec3(0, leftAngle, 0)
    );

    // Glass
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        mirrorColor,
        glm::vec3(-mX_Mirror + 0.01f, mY_Mirror, mZ - 0.05f),
        glassScale,
        glm::vec3(0, leftAngle, 0)
    );

    // Right side (looking at the front of the car)
    // Neck
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(mX_Neck + 0.075f, mY_Neck, mZ),
        neckScale
    );

    // Housing
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(mX_Housing, mY_Housing, mZ),
        houseScale,
        glm::vec3(0, rightAngle, 0)
    );

    // Glass
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        mirrorColor,
        glm::vec3(mX_Mirror - 0.01f, mY_Mirror, mZ - 0.05f),
        glassScale,
        glm::vec3(0, rightAngle, 0)
    );
}

void CivilianCarActive::CreateGrille(const std::weak_ptr<Shader>& shader, const glm::vec3& grilleBgColor, const glm::vec3& grilleColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float gY = 0.64f;
    constexpr float gZ = 2.41f;
    constexpr float gWidth = 0.845f;
    constexpr float gHeight = 0.32f;

    // Void
    float grilleBgY = gY + (gHeight * 0.5f); // 0.64 + 0.16 = 0.80f

    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
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
        AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
            grilleColor,
            glm::vec3(-0.3f + xOffset, barY, gZ + 0.01f),
            glm::vec3(0.05f, barHeight, 0.04f)
        );
    }
}

void CivilianCarActive::CreateCabinSide(const std::weak_ptr<Shader>& shader, FacePosition side, uint8_t alpha, const glm::vec3& bodyColor, const glm::vec3& windowColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    float multplier = HasFlag(side, FacePosition::Left) ? -1.0f : 1.0f;
    // Side window
    AddWindowPart(shader, WindowType::Flat,
        alpha,
        windowColor,
        glm::vec3(0.99f * multplier, 1.195f, -0.34f),
        glm::vec3(0.02f, 0.35f, 1.5f)
    );

    // Triangle windows
    // Rear Triangle
    AddWindowPart(shader, WindowType::Triangle,
        alpha,
        windowColor,
        glm::vec3(0.99f * multplier, 1.20f, -1.36f),
        glm::vec3(0.02f, 0.35f, 0.5f),
        glm::vec3(0.0f, glm::radians(180.0f), 0.0f)
    );

    // Front Triangle
    AddWindowPart(shader, WindowType::Triangle,
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
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(0.97f * multplier, pillarY, -1.10f),
        pillarScale,
        pillarRot
    );

    // Front Side Pillar
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(0.97f * multplier, pillarY, 0.4f),
        pillarScale,
        pillarRot
    );

    // Center Side Pillar
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        bodyColor,
        glm::vec3(0.97f * multplier, pillarY, -0.3f),
        pillarScale,
        pillarRot
    );
}

void CivilianCarActive::CreateFrontLights(const std::weak_ptr<Shader>& shader, const glm::vec3& shortBeamColor, const glm::vec3& highBeamColor, const glm::vec3& blinkerColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    // Left Front Light Cluster
    // LOOKING AT THE CAR'S FRONT
    constexpr float clusterX = -0.75f;
    constexpr float clusterY = 0.8f;
    constexpr float clusterZ = 2.38f;

    constexpr float shift = 0.05f;

    // Upper Left short beam
    AddSpotLightPart(shader, CivilianCarLightType::Headlight, FacePosition::Left, CivilianCarPartIndex::HeadLight_L, ShadowMapIndex::CV_Headlight_L,
        shortBeamColor,
        glm::vec3(clusterX, clusterY + 0.1f + shift, clusterZ),
        glm::vec3(0.5f, 0.1f, 0.1f)
    );

    // Bottom left high beam
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        highBeamColor,
        glm::vec3(clusterX + 0.125f, clusterY + shift, clusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Bottom left blinker
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        blinkerColor,
        glm::vec3(clusterX - 0.125f, clusterY + shift, clusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Right Front Light Cluster
    // Upper right short beam
    AddSpotLightPart(shader, CivilianCarLightType::Headlight, FacePosition::Right, CivilianCarPartIndex::HeadLight_R, ShadowMapIndex::CV_Headlight_R,
        shortBeamColor,
        glm::vec3(-clusterX, clusterY + 0.1f + shift, clusterZ),
        glm::vec3(0.5f, 0.1f, 0.1f)
    );

    // Bottom right high beam
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        highBeamColor,
        glm::vec3(-clusterX - 0.125f, clusterY + shift, clusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Bottom right blinker
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        blinkerColor,
        glm::vec3(-clusterX + 0.125f, clusterY + shift, clusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );
}

void CivilianCarActive::CreateBackLights(const std::weak_ptr<Shader>& shader, const glm::vec3& brakeLightColor, const glm::vec3& reverLightColor, const glm::vec3& blinkerColor, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float rClusterX = -0.75f;
    constexpr float rClusterY = 0.8f;
    constexpr float rClusterZ = -2.38f;

    constexpr float rShift = 0.05f;

    // LEFT SIDE CLUSTER (LOOKING AT THE FRONT OF CAR!!!)
    // Top left brake light
    AddSpotLightPart(shader, CivilianCarLightType::Rearlight, FacePosition::Left, CivilianCarPartIndex::RearLight_L, ShadowMapIndex::CV_Rearlight_L,
        brakeLightColor,
        glm::vec3(rClusterX, rClusterY + 0.1f + rShift, rClusterZ),
        glm::vec3(0.5f, 0.1f, 0.1f)
    );

    // Bottom left reverse light
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        reverLightColor,
        glm::vec3(rClusterX + 0.125f, rClusterY + rShift, rClusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Bottom left blinker light
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        blinkerColor,
        glm::vec3(rClusterX - 0.125f, rClusterY + rShift, rClusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // RIGHT SIDE CLUSTER
    // Top right brake light
    AddSpotLightPart(shader, CivilianCarLightType::Rearlight, FacePosition::Right, CivilianCarPartIndex::RearLight_R, ShadowMapIndex::CV_Rearlight_R,
        brakeLightColor,
        glm::vec3(-rClusterX, rClusterY + 0.1f + rShift, rClusterZ),
        glm::vec3(0.5f, 0.1f, 0.1f)
    );

    // Bottom right reverse light
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        reverLightColor,
        glm::vec3(-rClusterX - 0.125f, rClusterY + rShift, rClusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );

    // Bottom left blinker light
    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
        blinkerColor,
        glm::vec3(-rClusterX + 0.125f, rClusterY + rShift, rClusterZ),
        glm::vec3(0.25f, 0.1f, 0.1f)
    );
}

void CivilianCarActive::CreateSteeringWheel(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
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

    AddBodyPart(shader, CivilianCarMeshType::Body, CivilianCarPartIndex::None,
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
    CreatePart(std::move(mesh), CivilianCarMeshType::Steering, CivilianCarPartIndex::None);
}

void CivilianCarActive::AddBodyPart(const std::weak_ptr<Shader>& shader, CivilianCarMeshType type, CivilianCarPartIndex index, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);
    CreatePart(std::move(mesh), type, index, pos);
}

void CivilianCarActive::AddSpotLightPart(const std::weak_ptr<Shader>& shader, CivilianCarLightType lType, FacePosition side, CivilianCarPartIndex index, ShadowMapIndex shadowMapindex, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    CivilianCarPart part = CreateBaseLightPart(shader, lType, side, color, pos, scale, rot);

    SpotLight spot;
    spot.base.baseColor = color;
    spot.shadowMapIdx = shadowMapindex;
    spot.base.intensity = 0.0f;
    spot.direction = glm::vec3(0, 0, 1);

    switch (lType)
    {
        case CivilianCarLightType::Headlight:
        {
            spot.base.intensity = 5.0f;
            part.meshData->lightMeshIntensity = 1.0f;
            break;
        }
        case CivilianCarLightType::Rearlight:
        {
            spot.base.intensity = 2.0f;
            spot.direction = glm::vec3(0, 0, -1);
            spot.base.baseColor = glm::vec3(1.0f, 0.0, 0.0f);
            break;
        }
        default: break;
    }

    part.lightData->spotLight = std::make_unique<SpotLight>(spot);

    carPartIndex[static_cast<size_t>(index)] = parts.size();
    parts.push_back(std::move(part));
}

void CivilianCarActive::AddWheel(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    Transform t{ .position = pos, .rotation = glm::vec3(0, 0, glm::radians(90.0f)) + rot, .scale = scale };
    auto mesh = std::make_unique<CylinderMesh>(shader, 0.5f, 1.0f, 32, t, color);

    CreatePart(std::move(mesh), CivilianCarMeshType::Wheel, CivilianCarPartIndex::None, pos);
}

void CivilianCarActive::AddWindowPart(const std::weak_ptr<Shader>& shader, WindowType type, uint8_t alpha, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
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

    CivilianCarPart& part = CreatePart(std::move(mesh), CivilianCarMeshType::Window, CivilianCarPartIndex::None, pos);
}

glm::mat4 CivilianCarActive::CalculateWheelMatrix(const CivilianCarPart& part) const
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, part.meshData->offset);

    // Steering (only for front wheels)
    if (part.meshData->offset.z > 0)
    {
        m = glm::rotate(m, part.meshData->mesh->GetTransform().rotation.y, glm::vec3(0, 1, 0));
    }

    // Constant Wheel orientation + Rolling rotation
    m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1));
    m = glm::rotate(m, part.meshData->mesh->GetTransform().rotation.x, glm::vec3(0, 1, 0));
    m = glm::scale(m, part.meshData->mesh->GetTransform().scale);
    return m;
}

void CivilianCarActive::UpdatePhysics(double deltaTime)
{
    float dt = static_cast<float>(deltaTime);

    if (isBraking || std::abs(steerAngle) > 20.0f)
    {
        speed -= DECELERATION * dt;
    }
    else
    {
        speed -= (DECELERATION * 0.1f) * dt;
    }

    if (speed < 0.0f) speed = 0.0f;

    speed = glm::clamp(speed, -MAX_SPEED / 2.0f, MAX_SPEED);
    if (std::abs(speed) > MOVING_THRESHOLD)
    {
        // Raw turning change
        float steeringSensitivity = 1.0f - glm::clamp(std::abs(speed) / MAX_SPEED, 0.0f, 0.7f);
        float effectiveSteerAngle = steerAngle * steeringSensitivity;
        float turningCurvature = tan(glm::radians(effectiveSteerAngle)) / WHEEL_BASE;
        float rotationStep = glm::degrees(turningCurvature * speed * dt);

        currentAngle += rotationStep;

        // Lean (~centrifugal force)
        float turnIntensity = (rotationStep / dt) * (speed / MAX_SPEED);
        float targetRoll = -turnIntensity * 0.05f; // Negative because car leans OUT of the turn
        targetRoll = glm::clamp(targetRoll, -MAX_ROLL, MAX_ROLL);

        // Smoothly interpolate the roll so it doesn't snap
        visualRoll = glm::mix(visualRoll, targetRoll, dt * ROLL_STIFFNESS);
    }
    else
    {
        // Spring back to center when stopped
        visualRoll = glm::mix(visualRoll, 0.0f, dt * ROLL_STIFFNESS);
    }

    transform.position.x += sin(glm::radians(currentAngle)) * speed * dt;
    transform.position.z += cos(glm::radians(currentAngle)) * speed * dt;
    transform.rotation.y = glm::radians(currentAngle);
    transform.rotation.z = glm::radians(visualRoll);

    float targetPitch = (speed > 0 && isBraking) ? 2.0f : 0.0f;
    transform.rotation.x = glm::mix(transform.rotation.x, glm::radians(targetPitch), dt * 5.0f);

    wheelRotation += speed * dt * WHEEL_ROTATION_FACTOR;
}

glm::vec3 CivilianCarActive::GridToWorld(const glm::ivec2& gridPos) const
{
    float tileSize = 16.0f;
    float offsetX = (25.0f * tileSize) / 2.0f;
    float offsetZ = (25.0f * tileSize) / 2.0f;

    return glm::vec3(
        (gridPos.x * tileSize) - offsetX,
        0.0f,
        (gridPos.y * tileSize) - offsetZ
    );
}

glm::ivec2 CivilianCarActive::WorldToGrid(const glm::vec3& worldPos) const
{
    float tileSize = 16.0f;
    int gridSize = 25;

    float halfCitySize = (gridSize * tileSize) / 2.0f;

    // Shift the world coordinates so (0,0,0) becomes the center of the grid
    int x = static_cast<int>((worldPos.x + halfCitySize) / tileSize);
    int z = static_cast<int>((worldPos.z + halfCitySize) / tileSize);

    // Clamp values to ensure we don't access outside the matrix bounds
    return glm::ivec2(
        glm::clamp(x, 0, gridSize - 1),
        glm::clamp(z, 0, gridSize - 1)
    );
}

glm::ivec2 CivilianCarActive::FindFleeTarget(glm::ivec2 playerGrid, const std::vector<std::vector<CityTileType>>& grid)
{
    glm::ivec2 bestTile = { 0, 0 };
    float bestScore = -1000000.0f;

    glm::vec2 currentPosGrid = glm::vec2(WorldToGrid(transform.position));
    glm::vec2 currentDir(sin(glm::radians(currentAngle)), cos(glm::radians(currentAngle)));

    for (int y = 0; y < 25; ++y)
    {
        for (int x = 0; x < 25; ++x)
        {
            // Only drive on roads/grass (avoid buildings)
            if (grid[y][x] != CityTileType::Park && grid[y][x] != CityTileType::Skyscraper)
            {
                glm::vec2 tilePos(x, y);
                float distToPlayer = glm::distance(tilePos, glm::vec2(playerGrid));

                glm::vec2 toTileVec = tilePos - currentPosGrid;
                float distToTile = glm::length(toTileVec);
                if (distToTile < 0.1f)
                {
                    continue;
                }

                // Get direction
                glm::vec2 dirToTile = toTileVec / distToTile;
                float alignment = glm::dot(currentDir, dirToTile);

                // If the tile is behind us, reduce the score
                float alignmentMultiplier = (alignment < 0.0f) ? 0.2f : 1.0f;

                // Add a bit of bonus for being straight ahead
                float score = (distToPlayer * alignmentMultiplier) + (alignment * 5.0f);

                if (score > bestScore)
                {
                    bestScore = score;
                    bestTile = { x, y };
                }
            }
        }
    }
    return bestTile;
}

std::vector<glm::ivec2> CivilianCarActive::CalculatePath(glm::ivec2 start, glm::ivec2 target, const std::vector<std::vector<CityTileType>>& grid)
{
    if (start == target) return {};

    std::queue<glm::ivec2> frontier;
    frontier.push(start);

    // key: current_tile, value: parent_tile (to reconstruct path)
    // Hash: x << 16 | y
    std::unordered_map<int, glm::ivec2> cameFrom;
    cameFrom[start.x << 16 | start.y] = start;

    bool found = false;
    std::vector<glm::ivec2> neighbors = { {0,1}, {0,-1}, {1,0}, {-1,0} };

    while (!frontier.empty())
    {
        glm::ivec2 current = frontier.front();
        frontier.pop();

        if (current == target) {
            found = true;
            break;
        }

        for (const auto& offset : neighbors)
        {
            glm::ivec2 next = current + offset;

            // Bounds check
            if (next.x >= 0 && next.x < 25 && next.y >= 0 && next.y < 25)
            {
                int nextKey = next.x << 16 | next.y;
                // Check if it's a road we didn't visit yet
                if ((grid[next.y][next.x] != CityTileType::Park && grid[next.y][next.x] != CityTileType::Skyscraper) && cameFrom.find(nextKey) == cameFrom.end())
                {
                    frontier.push(next);
                    cameFrom[nextKey] = current;
                }
            }
        }
    }

    // Reconstruct the path backwards from target to start
    std::vector<glm::ivec2> finalPath;
    if (found)
    {
        glm::ivec2 curr = target;
        while (curr != start)
        {
            finalPath.push_back(curr);
            curr = cameFrom[curr.x << 16 | curr.y];
        }
        std::reverse(finalPath.begin(), finalPath.end());
    }

    return finalPath;
}

void CivilianCarActive::UpdateWheelTransform(CivilianCarPart& part) const
{
    Transform t = part.meshData->mesh->GetTransform();

    t.rotation.x = wheelRotation;

    // ONLY front wheels steer (offset.z > 0)
    if (part.meshData->offset.z > 0)
    {
        t.rotation.y = glm::radians(steerAngle);
    }
    else
    {
        t.rotation.y = 0.0f;
    }

    part.meshData->mesh->SetTransform(t);
}

void CivilianCarActive::UpdateSteeringTransform(CivilianCarPart& part) const
{
    Transform t = part.meshData->mesh->GetTransform();
    t.rotation.y = glm::radians(glm::clamp(steerAngle, -120.0f, 120.0f));
    part.meshData->mesh->SetTransform(t);
}

void CivilianCarActive::HandleSpotLight(CivilianCarPart& part) const
{
    if (!part.lightData || !part.lightData->spotLight)
    {
        return;
    }

    auto& spot = part.lightData->spotLight;
    auto& light = part.lightData;

    float intensity = 0.0f;

    if (light->type == CivilianCarLightType::Rearlight)
    {
        intensity = (isBraking || speed == 0.0f) ? BRAKE_LIGHT_USE_INTENSITY : BRAKE_LIGHT_IDLE_INTENSITY;
    }
    else
    {
        intensity = 1.0f;
    }

    spot->base.position = TransformOffsetToWorld(light->offset);
    spot->base.intensity = intensity;
    spot->base.currentColor = spot->base.baseColor;

    if (part.meshData)
    {
        if (light->type == CivilianCarLightType::Rearlight)
        {
            part.meshData->lightMeshIntensity = (intensity > BRAKE_LIGHT_IDLE_INTENSITY) ? 1.0f : 0.5f;
        }
        else
        {
            part.meshData->lightMeshIntensity = 1.0f;
        }
    }

    float zDir = (light->type == CivilianCarLightType::Rearlight) ? -1.0f : 1.0f;
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), transform.rotation.y, glm::vec3(0, 1, 0));
    spot->direction = glm::vec3(rotation * glm::vec4(0.0f, 0.0f, zDir, 0.0f));
}

void CivilianCarActive::Bake(const std::weak_ptr<Shader>& shader)
{
    data->bakedBodyMesh.reset();
    data->bakedAlphaMesh.reset();
    data->bakedLightMesh.reset();

    data->bakedBodyMesh = PerformBakePass(shader, BakePassType::Solid);
    data->bakedAlphaMesh = PerformBakePass(shader, BakePassType::Alpha);
    data->bakedLightMesh = PerformBakePass(shader, BakePassType::Light);

    if (data->bakedAlphaMesh && data->windowTex)
    {
        data->bakedAlphaMesh->SetTexture(data->windowTex);
    }
}

std::unique_ptr<Mesh> CivilianCarActive::PerformBakePass(const std::weak_ptr<Shader>& shader, BakePassType passType)
{
    std::vector<float> allVertices;
    std::vector<unsigned int> allIndices;

    std::vector<MeshCache> batches;
    batches.reserve(parts.size());

    size_t totalVertFloats = 0;
    size_t totalIndices = 0;

    dynamicPartIndices.clear();

    for (size_t i = 0; i < parts.size(); ++i)
    {
        auto& part = parts[i];
        if (!part.meshData || !part.meshData->mesh)
        {
            continue;
        }

        if (part.meshData->type == CivilianCarMeshType::Wheel ||
            part.meshData->type == CivilianCarMeshType::Light ||
            part.meshData->type == CivilianCarMeshType::Steering)
        {
            dynamicPartIndices.push_back(i);
            continue;
        }

        bool isWindow = (part.meshData->type == CivilianCarMeshType::Window);

        if (passType == BakePassType::Alpha && !isWindow) continue;
        if (passType == BakePassType::Solid && isWindow) continue;

        auto& mesh = part.meshData->mesh;
        MeshCache batch;
        batch.verts = mesh->GetVertexData();
        batch.inds = mesh->GetIndexData();
        batch.transform = mesh->GetTransform().GetModelMatrix();

        totalVertFloats += batch.verts.size();
        totalIndices += batch.inds.size();

        batches.push_back(std::move(batch));
    }

    allVertices.reserve(totalVertFloats);
    allIndices.reserve(totalIndices);

    for (const auto& batch : batches)
    {
        // vertOffset is the count of vertices currently in the buffer
        const unsigned int vertOffset = static_cast<unsigned int>(allVertices.size() / STRIDE);

        glm::mat3 normMat = glm::mat3(glm::transpose(glm::inverse(batch.transform)));

        for (size_t v = 0; v < batch.verts.size(); v += STRIDE)
        {
            // Transform Position
            glm::vec4 pos = batch.transform * glm::vec4(batch.verts[v], batch.verts[v + 1], batch.verts[v + 2], 1.0f);
            allVertices.push_back(pos.x);
            allVertices.push_back(pos.y);
            allVertices.push_back(pos.z);

            // Transform Normal
            glm::vec3 norm = glm::normalize(normMat * glm::vec3(batch.verts[v + 3], batch.verts[v + 4], batch.verts[v + 5]));
            allVertices.push_back(norm.x);
            allVertices.push_back(norm.y);
            allVertices.push_back(norm.z);

            // Bulk copy UVs, Tangents, and Colors
            allVertices.insert(allVertices.end(),
                batch.verts.begin() + v + 6,
                batch.verts.begin() + v + 14
            );
        }

        // Offset indices so they point to the correct vertices in the new global buffer
        for (auto idx : batch.inds)
        {
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

