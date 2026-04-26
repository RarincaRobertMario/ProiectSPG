#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "PoliceCar.h"
#include "CubeMesh.h"
#include "CylinderMesh.h"
#include "PoliceCarModel.h"
#include "PoliceCarActive.h"

PoliceCar::PoliceCar(const std::weak_ptr<Shader>& shader, const Transform& transform) :
    transform(transform) 
{
    Init(shader);
    Bake(shader);
}

void PoliceCar::Update(double deltaTime)
{
    float dt = static_cast<float>(deltaTime);

    // Apply Friction /  acceleration
    if (speed > 0) speed = std::max(0.0f, speed - deceleration * dt);
    else if (speed < 0) speed = std::min(0.0f, speed + deceleration * dt);

    // 2. Clamp to absolute stop if it's crawling (Prevents "Ghost Creeping")
    if (glm::abs<float>(speed) < 0.01f) {
        speed = 0.0f;
    }

    speed = glm::clamp(speed, -maxSpeed / 2.0f, maxSpeed);

    // 3. Logic: If speed is 0, the brake lights MUST be on (Idling at a light)
    // We check against 0.0f directly now because we clamped it above.
    bool isStationary = speed == 0.0f;

    // 2. Determine if lights should be bright (Braking OR Stationary Idling)
    // This captures your "foot on brake while stopped" preference
    bool showBrakeLights = isBraking || isStationary;

    // 3. Update the actual Rear Light components
    auto itL = carPartIndex[static_cast<size_t>(PoliceCarPartIndex::RearLight_L)];
    auto itR = carPartIndex[static_cast<size_t>(PoliceCarPartIndex::RearLight_R)];

    if (itL != -1 && itR != -1)
    {
        auto& partL = parts[itL];
        auto& partR = parts[itR];

        float meshInt = 0.0f;
        float lightInt = 0.0f;

        if (areBrakeLightsOn)
        {
            // If showBrakeLights is true, use high intensity (1.0 mesh, 8.0 light)
            // Otherwise, use "Tail Light" mode (0.5 mesh, 2.0 light)
            meshInt = showBrakeLights ? 1.0f : 0.5f;
            lightInt = showBrakeLights ? 8.0f : 2.0f;
        }


        partL.meshData->lightMeshIntensity = meshInt;
        partL.lightData->spotLight->base.intensity = lightInt;

        partR.meshData->lightMeshIntensity = meshInt;
        partR.lightData->spotLight->base.intensity = lightInt;
    }


    if (std::abs(speed) > 0.1f) 
    {
        float wheelbase = 2.5f; // Distance between front and back wheels
        float turningCurvature = tan(glm::radians(steerAngle)) / wheelbase;
        currentAngle += glm::degrees(turningCurvature * speed * dt);
    }

    // 4. Movement & Rotation
    transform.position.x += sin(glm::radians(currentAngle)) * speed * dt;
    transform.position.z += cos(glm::radians(currentAngle)) * speed * dt;
    transform.rotation.y = glm::radians(currentAngle);

    // Wheel Animation
    wheelRotation += speed * dt * 8.0f; // Multiplier controls 'rolling' speed

    // Update Parts and Lights
    sirenTimer += deltaTime;
    blinkerTimer += deltaTime;
    for (auto& part : parts)
    {
        if (part.lightData) {
            if (part.lightData->pointLight)
            {
                HandlePointLight(part);
            }
            if (part.lightData->spotLight) 
            {
                HandleSpotLight(part);
            }
        }

        if (part.meshData && part.meshData->type == PoliceCarMeshType::Wheel)
        {
            Transform t = part.meshData->mesh->GetTransform();

            // EVERY wheel needs to roll
            t.rotation.x = wheelRotation;

            // ONLY front wheels steer
            if (part.meshData->offset.z > 0) 
            {
                t.rotation.y = glm::radians(steerAngle);
            }
            else {
                t.rotation.y = 0.0f; // Back wheels stay straight
            }

            part.meshData->mesh->SetTransform(t);
        }
    }
}

void PoliceCar::Draw(std::shared_ptr<Shader> customShader) const
{
    //glm::mat4 carMatrix = transform.GetModelMatrix();
    //for (auto& part : parts)
    //{
    //    if (!part.meshData) continue;

    //    if (customShader && !part.meshData->castShadow)
    //    {
    //        continue;
    //    }

    //    auto& mesh = part.meshData->mesh;

    //    // Use the custom shader if provided, otherwise the mesh's own shader
    //    auto activeShader = customShader ? customShader : mesh->GetShader().lock();

    //    if (activeShader)
    //    {
    //        // ONLY set these if we aren't doing the shadow pass 
    //        // (Shadow shaders usually don't care about 'isLightMesh')
    //        GLint isLightLoc = activeShader->GetUniformLocation("isLightMesh");
    //        if (isLightLoc != -1)
    //        {
    //            glUniform1i(isLightLoc, (part.meshData->type == PoliceCarMeshType::Light) ? 1 : 0);
    //            float intensity = (part.isActivated) ? part.meshData->lightMeshIntensity : 0.05f;
    //            glUniform1f(activeShader->GetUniformLocation("lightMeshIntensity"), intensity);
    //        }

    //        glm::mat4 partLocalMatrix;
    //        if (part.meshData->type == PoliceCarMeshType::Wheel)
    //        {
    //            partLocalMatrix = glm::mat4(1.0f);
    //            partLocalMatrix = glm::translate(partLocalMatrix, part.meshData->offset);
    //            if (part.meshData->offset.z > 0) {
    //                partLocalMatrix = glm::rotate(partLocalMatrix, mesh->GetTransform().rotation.y, glm::vec3(0, 1, 0));
    //            }
    //            partLocalMatrix = glm::rotate(partLocalMatrix, glm::radians(90.0f), glm::vec3(0, 0, 1));
    //            partLocalMatrix = glm::rotate(partLocalMatrix, mesh->GetTransform().rotation.x, glm::vec3(0, 1, 0));
    //            partLocalMatrix = glm::scale(partLocalMatrix, mesh->GetTransform().scale);
    //        }
    //        else
    //        {
    //            partLocalMatrix = mesh->GetTransform().GetModelMatrix();
    //        }

    //        glm::mat4 finalModelMatrix = carMatrix * partLocalMatrix;

    //        // CRITICAL: Use the location from the ACTIVE shader
    //        glUniformMatrix4fv(activeShader->GetUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(finalModelMatrix));
    //        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(finalModelMatrix)));
    //        glUniformMatrix3fv(activeShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

    //        mesh->DrawRaw(customShader);

    //        if (isLightLoc != -1) glUniform1i(isLightLoc, 0);
    //    }
    //}

    glm::mat4 carMatrix = transform.GetModelMatrix();
    auto activeShader = customShader ? customShader : bakedBodyMesh->GetShader().lock();

    if (!activeShader) return;

    // --- STEP 1: Draw the entire baked body (1 Call!) ---
    glUniformMatrix4fv(activeShader->GetUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(carMatrix));
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(carMatrix)));
    glUniformMatrix3fv(activeShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

    // Set intensity for non-light parts
    glUniform1i(activeShader->GetUniformLocation("isLightMesh"), 0);
    bakedBodyMesh->DrawRaw(customShader);

    for (size_t idx : dynamicPartIndices)
    {
        auto& part = parts[idx];
        if (!part.meshData)
        {
            continue;
        }

        if (customShader && !part.meshData->castShadow)
        {
            continue;
        }

        auto& mesh = part.meshData->mesh;

        // Use the custom shader if provided, otherwise the mesh's own shader
        auto activeShader = customShader ? customShader : mesh->GetShader().lock();

        if (activeShader)
        {
            // ONLY set these if we aren't doing the shadow pass 
            // (Shadow shaders usually don't care about 'isLightMesh')
            GLint isLightLoc = activeShader->GetUniformLocation("isLightMesh");
            if (isLightLoc != -1)
            {
                glUniform1i(isLightLoc, (part.meshData->type == PoliceCarMeshType::Light) ? 1 : 0);
                float intensity = (part.isActivated) ? part.meshData->lightMeshIntensity : 0.05f;
                glUniform1f(activeShader->GetUniformLocation("lightMeshIntensity"), intensity);
            }

            glm::mat4 partLocalMatrix;
            if (part.meshData->type == PoliceCarMeshType::Wheel)
            {
                partLocalMatrix = glm::mat4(1.0f);
                partLocalMatrix = glm::translate(partLocalMatrix, part.meshData->offset);
                if (part.meshData->offset.z > 0) {
                    partLocalMatrix = glm::rotate(partLocalMatrix, mesh->GetTransform().rotation.y, glm::vec3(0, 1, 0));
                }
                partLocalMatrix = glm::rotate(partLocalMatrix, glm::radians(90.0f), glm::vec3(0, 0, 1));
                partLocalMatrix = glm::rotate(partLocalMatrix, mesh->GetTransform().rotation.x, glm::vec3(0, 1, 0));
                partLocalMatrix = glm::scale(partLocalMatrix, mesh->GetTransform().scale);
            }
            else
            {
                partLocalMatrix = mesh->GetTransform().GetModelMatrix();
            }

            glm::mat4 finalModelMatrix = carMatrix * partLocalMatrix;

            // CRITICAL: Use the location from the ACTIVE shader
            glUniformMatrix4fv(activeShader->GetUniformLocation("modelMatrix"), 1, GL_FALSE, glm::value_ptr(finalModelMatrix));
            glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(finalModelMatrix)));
            glUniformMatrix3fv(activeShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

            mesh->DrawRaw(customShader);

            if (isLightLoc != -1) glUniform1i(isLightLoc, 0);
        }
    }
}

void PoliceCar::ProcessKeyboard(Direction direction, float deltaTime)
{
    float dt = deltaTime;

    if (direction == Direction::Forward)
    {
        speed += acceleration * dt;
    }
    else if (direction == Direction::Backward)
    {
        speed -= acceleration * dt;
    }

    isBraking = (speed > 0.1f) && direction == Direction::Backward;
    isReversing = (speed < -0.1f);

    auto it = carPartIndex[static_cast<size_t>(PoliceCarPartIndex::ReverseLight_L)];
    if (it != -1)
    {
        auto& part = parts[it];
        part.meshData->lightMeshIntensity = isReversing ? 1.0f : 0.0f;
        part.isActivated = isReversing;
        part.lightData->spotLight->base.intensity = isReversing ? 4.0f : 0.0f;
    }

    it = carPartIndex[static_cast<size_t>(PoliceCarPartIndex::ReverseLight_R)];
    if (it != -1)
    {
        auto& part = parts[it];
        part.meshData->lightMeshIntensity = isReversing ? 1.0f : 0.0f;
        part.isActivated = isReversing;
        part.lightData->spotLight->base.intensity = isReversing ? 4.0f : 0.0f;
    }

    if (std::abs(speed) > 0.1f)
    {
        if (direction == Direction::Left)
        {
            steerAngle = glm::mix(steerAngle, 30.0f, dt * 5.0f);
            return;
        }
        else if (direction == Direction::Right)
        {
            steerAngle = glm::mix(steerAngle, -30.0f, dt * 5.0f);
            return;
        }
        steerAngle = glm::mix(steerAngle, 0.0f, dt * 5.0f);
    }
}

void PoliceCar::ToggleShortBeams()
{
    static bool enabled = false;
    auto it = carPartIndex[static_cast<size_t>(PoliceCarPartIndex::HeadLight_L)];
    if (it != -1)
    {
        auto& part = parts[it];
        part.isActivated = enabled;
        part.lightData->spotLight->base.intensity = enabled ? 5.0f : 0.0f;
    }

    it = carPartIndex[static_cast<size_t>(PoliceCarPartIndex::HeadLight_R)];
    if (it != -1)
    {
        auto& part = parts[it];
        part.isActivated = enabled;
        part.lightData->spotLight->base.intensity = enabled ? 5.0f : 0.0f;
    }

    enabled = !enabled;
}

void PoliceCar::ToggleHighBeams()
{
    static bool enabled = true;
    auto it = carPartIndex[static_cast<size_t>(PoliceCarPartIndex::HighBeam_L)];
    if (it != -1)
    {
        auto& part = parts[it];
        part.isActivated = enabled;
        part.lightData->spotLight->base.intensity = enabled ? 10.0f : 0.0f;
    }

    it = carPartIndex[static_cast<size_t>(PoliceCarPartIndex::HighBeam_R)];
    if (it != -1)
    {
        auto& part = parts[it];
        part.isActivated = enabled;
        part.lightData->spotLight->base.intensity = enabled ? 10.0f : 0.0f;
    }

    enabled = !enabled;
}

void PoliceCar::SetBlinkerState(BlinkerSide side)
{
    if (blinkerSide == side) 
    {
        blinkerSide = BlinkerSide::None;
    }
    else 
    {
        blinkerSide = side;
    }

    blinkerTimer = 0.0;
}

void PoliceCar::ToggleLightBar()
{
    isLightBarOn = !isLightBarOn;
    sirenTimer = 0.0;
}

void PoliceCar::ToggleRearLights() { areBrakeLightsOn = !areBrakeLightsOn; }

std::vector<PoliceCarLight *> PoliceCar::GetAllLights() 
{
    std::vector<PoliceCarLight*> lights = {};
    for (auto& part : parts)
    {
        if (part.lightData)
        {
            lights.push_back(part.lightData.get());
        }
    }
    return lights; 
}

glm::vec3 PoliceCar::GetForward() const
{
    // Assuming your car starts facing +Z. Adjust if it faces -Z or +X.
    return glm::vec3(sin(glm::radians(currentAngle)), 0.0f, cos(glm::radians(currentAngle)));
}

glm::vec3 PoliceCar::GetWorldPosOfLight(PoliceCarPartIndex partIndex) const
{
    auto it = carPartIndex[static_cast<size_t>(partIndex)];
    if (it == -1)
    {
        return transform.position;
    }

    auto& part = parts[it];

    // CRITICAL FIX: Use lightData offset if it exists!
    glm::vec3 localOffset = (part.lightData) ? part.lightData->offset : part.meshData->offset;

    // Use the same rotation logic as HandlePointLight for 1:1 parity
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), transform.rotation.y, glm::vec3(0, 1, 0));

    glm::vec3 worldOffset = glm::vec3(rotation * glm::vec4(localOffset, 1.0f));
    return transform.position + worldOffset;
}

float PoliceCar::GetSpotlightOuterAngle(PoliceCarPartIndex partIndex) const
{
    auto it = carPartIndex[static_cast<size_t>(partIndex)];
    if (it == -1)
    {
        return 1.0f;
    }

    // Safety check to ensure the vector hasn't changed in an unexpected way
    if (static_cast<size_t>(it) >= parts.size()) return 1.0f;

    auto& part = parts[it];

    if (part.lightData && part.lightData->spotLight)
    {
        return part.lightData->spotLight->outerCutOff;
    }

    return 1.0f;
}

void PoliceCar::Init(const std::weak_ptr<Shader>& shader)
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
    constexpr glm::vec3 lightBarBoxColor = glm::vec3(0.2f);
    constexpr glm::vec3 wheelColor = glm::vec3(0.1f);
    constexpr glm::vec3 shortBeamColor = glm::vec3(1.0f);
    constexpr glm::vec3 highBeamColor = glm::vec3(1.0f, 1.0f, 0.8f);
    constexpr glm::vec3 blinkerColor = glm::vec3(1.0f, 0.5f, 0.0f);
    constexpr glm::vec3 plateColor = glm::vec3(0.85f);
    constexpr glm::vec3 grilleBgColor = glm::vec3(0.02f);
    constexpr glm::vec3 grilleBarColor = glm::vec3(0.25f);

    /* --- START MAIN BODY ---*/

        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::Body,
            bodyColor,
            glm::vec3(0, 0.45f, 0),
            true ? glm::vec3(2.2f, 0.6f, 4.8f) : glm::vec3(0.0f)
        );

    /* --- STOP MAIN BODY ---*/

    /* --- START TRUNK TAB --- */

        constexpr float tabWidth = 0.3f;
        constexpr float tabHeight = 0.1f;
        constexpr float tabDepth = 0.01f;
        constexpr float tabZ = -2.4f; 
        constexpr float tabY = 0.8f;

        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor,
            glm::vec3(0.0f, tabY, tabZ),
            glm::vec3(tabWidth, tabHeight, tabDepth)
        );

    /* --- STOP TRUNK TAB --- */

    /* --- START BUMPERS ---*/

        constexpr float bWidth = 2.3f;
        constexpr float bHeight = 0.25f;
        constexpr float bThick = 0.12f;
        constexpr float bY = 0.35f;
        constexpr float bZ_Front = 2.45f;
        constexpr float bZ_Rear = -2.45f;

        constexpr float sideLen = 0.4f;
        constexpr glm::vec3 sideBumperScale(0.08f, bHeight, sideLen);
        constexpr float sideX = (bWidth / 2.0f) - 0.040f;

        // Front bumper
        // Looking at the front of the car!
        // Front Face
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(0, bY, bZ_Front), 
            glm::vec3(bWidth, bHeight, bThick)
        );

        // Left Wrap
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(-sideX, bY, bZ_Front - (sideLen / 2.0f)), 
            sideBumperScale
        );

        // Right Wrap
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor,
            glm::vec3(sideX, bY, bZ_Front - (sideLen / 2.0f)), 
            sideBumperScale
        );

        // Rear bumper
        // Looking at the front of the car!
        // Rear Face
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(0, bY, bZ_Rear), 
            glm::vec3(bWidth, bHeight, bThick)
        );

        // Left Wrap
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(-sideX, bY, bZ_Rear + (sideLen / 2.0f)), 
            sideBumperScale
        );

        // Right Wrap
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(sideX, bY, bZ_Rear + (sideLen / 2.0f)), 
            sideBumperScale
        );

    /* --- STOP BUMPERS ---*/

    /* --- START LICENSE PLATES --- */

        constexpr glm::vec3 plateSize(0.55f, 0.15f, 0.01f);

        // Front Plate
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            plateColor,
            glm::vec3(0.0f, 0.4f, bZ_Front + 0.06f),
            plateSize
        );

        // Rear Plate 
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            plateColor,
            glm::vec3(0.0f, 0.4f, bZ_Rear - 0.06f),
            plateSize
        );

    /* --- STOP LICENSE PLATES --- */

    /* --- START SIDE SKIRTS --- */

        // Right side, looking at the car
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(1.1f, 0.35f, 0.0f),
            glm::vec3(0.05f, 0.15f, 2.1f)
        );

        // Left side.
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor,
            glm::vec3(-1.1f, 0.35f, 0.0f),
            glm::vec3(0.05f, 0.15f, 2.1f)
        );

    /* --- STOP SIDE SKIRTS --- */

    /* --- START DOOR HANDLES --- */

        constexpr float hX = 1.11f;
        constexpr float hY = 0.9f;
        constexpr float hZ_Front = -0.05f;
        constexpr float hZ_Rear = -0.9f;
        constexpr glm::vec3 handleScale(0.02f, 0.05f, 0.25f);

        // Left Front
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(-hX, hY, hZ_Front), 
            handleScale
        );
        // Right Front
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(hX, hY, hZ_Front), 
            handleScale
        );
        // Left Rear
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(-hX, hY, hZ_Rear), 
            handleScale
        );
        // Right Rear
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bumperColor, 
            glm::vec3(hX, hY, hZ_Rear), handleScale
        );

    /* --- STOP DOOR HANDLES --- */

    /* --- START FENDERS ---*/

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
                glm::vec3(x, y, z), 
                topScale
            );

            // Front slant (angled down, towards the front of the car)
            AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
                bodyColor,
                glm::vec3(x, y - 0.12f, z + 0.35f),
                sideScale,
                glm::vec3(slantAngle, 0, 0)
            );

            // Rear slang (angled down, towards the rear of the car)
            AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
                bodyColor,
                glm::vec3(x, y - 0.12f, z - 0.35f),
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

    /* --- STOP FENDERS ---*/

    /* --- START CABIN MAIN ---*/

        constexpr float windShieldAngle = 45.0f;
        constexpr float rearWindowAngle = 35.0f;

        // Looking at the car from the front
        // Windshield
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(0, 1.05f, 0.75f),
            glm::vec3(2.05f, 0.02f, 0.9f),
            glm::vec3(glm::radians(windShieldAngle), 0, 0)
        );

        // Roof
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,                  
            glm::vec3(0, 1.35f, -0.35f),
            glm::vec3(2.05f, 0.05f, 1.6f)
        );


        // Rear window
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(0, 1.15f, -1.45f),
            glm::vec3(2.00f, 0.02f, 0.77f),
            glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
        );

        // Front pillars
        // Looking at the car from the front
        // Left front pillar
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(-0.99f, 1.15f, 0.63f),
            glm::vec3(0.08f, 0.05f, 0.61f),
            glm::vec3(glm::radians(windShieldAngle), 0, 0)
        );

        // Right front pillar
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(0.99f, 1.15f, 0.63f),
            glm::vec3(0.08f, 0.05f, 0.62f),
            glm::vec3(glm::radians(windShieldAngle), 0, 0)
        );

        // Rear pillars
        // Looking at the car from the BACK !!!
        // Right rear pillar
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(-0.95f, 1.15f, -1.4f),
            glm::vec3(0.15f, 0.05f, 0.7f),
            glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
        );

        // Left rear pillar
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(0.95f, 1.15f, -1.4f),
            glm::vec3(0.15f, 0.05f, 0.7f),
            glm::vec3(glm::radians(-rearWindowAngle), 0, 0)
        );

    /* --- STOP CABIN MAIN ---*/

    /* --- START MIRRORS --- */

        constexpr float mX_Neck = 1.00f; 
        constexpr float mX_Housing = 1.23f;
        constexpr float mX_Mirror = mX_Housing;
        constexpr float mY_Neck = 1.05f;
        constexpr float mY_Housing = 1.02f;
        constexpr float mY_Mirror = mY_Housing + 0.01f;
        constexpr float mZ = 0.72f;

        constexpr glm::vec3 neckScale(0.2f, 0.05f, 0.05f);
        constexpr glm::vec3 houseScale(0.21f, 0.105f, 0.10f);
        constexpr glm::vec3 glassScale(0.16f, 0.09f, 0.01f);

        constexpr float leftAngle = glm::radians(-15.0f);
        constexpr float rightAngle = glm::radians(15.0f);

        // Left side
        // As in looking at the front of the car!
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
            windowColor,
            glm::vec3(-mX_Mirror + 0.01f, mY_Mirror, mZ - 0.05f),
            glassScale,
            glm::vec3(0, leftAngle, 0)
        );

        // Right side
        // As in looking at the front of the car.
        // Neck
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(mX_Neck + 0.075f, mY_Neck, mZ),
            neckScale
        );

        // housing
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(mX_Housing, mY_Housing, mZ),
            houseScale,
            glm::vec3(0, rightAngle, 0)
        );
        // Glass
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(mX_Mirror - 0.01f, mY_Mirror, mZ - 0.05f),
            glassScale,
            glm::vec3(0, rightAngle, 0)
        );

    /* --- STOP SIDE MIRRORS --- */

    /* --- START GRILLE --- */

        constexpr float gY = 0.64f;
        constexpr float gZ = 2.41f;
        constexpr float gWidth = 0.845f;
        constexpr float gHeight = 0.32f;

        // 'Black' behind the grills
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            grilleBgColor,
            glm::vec3(0.0f, gY, gZ),
            glm::vec3(gWidth, gHeight, 0.02f)
        );

        // Vertical bars.
        for (int i = 0; i < 9; i++) 
        {
            float xOffset = (i - 1) * 0.1f;
            AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
                grilleBarColor,
                glm::vec3(-0.3 + xOffset, gY, gZ + 0.01f),
                glm::vec3(0.05, gHeight + 0.01f, 0.04f)
            );
        }

    /* --- STOP GRILLE --- */

    /* --- START CABIN LEFT SIDE ---*/
        // Left side as in looking at the front of the car

        // Left side window
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(-0.99f, 1.05f, -0.35f),
            glm::vec3(0.02f, 0.3f, 1.5f)
        );
       
        // Triangles windows (direction as you're looking at the left side of the car)
        // Left side triangle window
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(-0.99f, 0.94f, -1.46f),
            glm::vec3(0.02f, 0.5f, 0.3f),
            glm::vec3(glm::radians(55.0f), 0.0f, 0.0f)
        );

        // Right side triangle window
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(-0.99f, 0.98f, 0.68f),
            glm::vec3(0.02f, 0.465f, 0.25f),
            glm::vec3(glm::radians(-45.0f), 0.0f, 0.0f)
        );

        // Side pillars (direction as you're looking at the left side of the car)
        // Left
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(-0.99f, 1.05f, -1.15f),
            glm::vec3(0.08f, 0.05f, 0.61f),
            glm::vec3(glm::radians(90.0f), 0, 0)
        );

        // Right
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(-0.99f, 1.05f, 0.4f), 
            glm::vec3(0.08f, 0.05f, 0.61f),
            glm::vec3(glm::radians(90.0f), 0, 0)
        );

        // Center
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(-0.99f, 1.05f, -0.3f),
            glm::vec3(0.08f, 0.05f, 0.61f),
            glm::vec3(glm::radians(90.0f), 0, 0)
        );


    /* --- STOP CABIN LEFT SIDE --- */

    /* --- START CABIN RIGHT SIDE --- */
        // Right side as looking at the car from the front

        // Right side window
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(0.99f, 1.05f, -0.35f), 
            glm::vec3(0.02f, 0.3f, 1.5f)
        );

        // Triangles windows (direction as you're looking at the right side of the car)
        // Right side triangle window
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(0.99f, 0.94f, -1.46f),
            glm::vec3(0.02f, 0.5f, 0.3f),
            glm::vec3(glm::radians(55.0f), 0.0f, 0.0f)
        );

        // Left side triangle window
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            windowColor,
            glm::vec3(0.99f, 0.98f, 0.68f),
            glm::vec3(0.02f, 0.465f, 0.25f),
            glm::vec3(glm::radians(-45.0f), 0.0f, 0.0f)
        );

        // Side pillars (direction as you're looking at the right side of the car)
        // Right
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(0.99f, 1.05f, -1.15f),
            glm::vec3(0.08f, 0.05f, 0.61f),
            glm::vec3(glm::radians(90.0f), 0, 0)
        );

        // Left
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor,
            glm::vec3(0.99f, 1.05f, 0.4f),
            glm::vec3(0.08f, 0.05f, 0.61f),
            glm::vec3(glm::radians(90.0f), 0, 0)
        );

        // Center
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None,
            bodyColor, 
            glm::vec3(0.99f, 1.05f, -0.3f),
            glm::vec3(0.08f, 0.05f, 0.61f),
            glm::vec3(glm::radians(90.0f), 0, 0)
        );

    /* --- STOP CABIN RIGHT SIDE --- */

    /* --- START LIGHTBAR --- */

        constexpr float lbY = true ? 1.4f : 2.5;
        constexpr float lbZ = -0.2f;
        constexpr float lbX = 0.55f;

        // Looking at the car from the front
        // Central Siren Box
        AddBodyPart(shader, PoliceCarMeshType::Body, PoliceCarPartIndex::None, 
            lightBarBoxColor,
            glm::vec3(0, lbY, lbZ), 
            glm::vec3(0.4f, 0.18f, 0.3f)
        );

        // Left light bar (red)
        AddPointLightPart(shader, PoliceCarLightType::LightBar, FacePosition::Left, PoliceCarPartIndex::LightBar_L, ShadowCubeIndex::Lightbar_L,
            glm::vec3(1, 0, 0), 
            glm::vec3(-lbX, lbY, lbZ),
            glm::vec3(0.8f, 0.15f, 0.25f)
        );

        // Right light bar (blue)
        AddPointLightPart(shader, PoliceCarLightType::LightBar, FacePosition::Right, PoliceCarPartIndex::LightBar_R, ShadowCubeIndex::Lightbar_R,
            glm::vec3(0, 0, 1), 
            glm::vec3(lbX, lbY, lbZ),
            glm::vec3(0.8f, 0.15f, 0.25f)
        );

    /* --- STOP LIGHTBAR --- */

    /* --- START WHEELS --- */

        constexpr float wX = 1.05f;
        constexpr float wY = 0.45f;
        constexpr float wZ = 1.5f;
        // THESE ARE THE 'PRE ROTATED' SCALES!!! AFTER ROTATION Y BECOMES X
        constexpr glm::vec3 wheelScale = glm::vec3(0.7f, 0.4f, 0.7f);

        // Looking at the car from the front.
        // Front left wheel
        AddWheel(shader, PoliceCarPartIndex::None, 
            wheelColor, 
            glm::vec3(-wX, wY,  wZ),
            wheelScale
        );

        // Front right wheel
        AddWheel(shader, PoliceCarPartIndex::None,
            wheelColor,
            glm::vec3( wX, wY,  wZ),
            wheelScale
        );

        // Back left wheel
        AddWheel(shader, PoliceCarPartIndex::None,
            wheelColor,
            glm::vec3(-wX, wY, -wZ), 
            wheelScale
        );

        // Back right wheel.
        AddWheel(shader, PoliceCarPartIndex::None,
            wheelColor,
            glm::vec3( wX, wY, -wZ), 
            wheelScale
        );

    /* --- STOP WHEELS --- */

    /* --- START FRONT LIGHTS --- */

        // Left Front Light Cluster
        // Looking at the car from the front.
        constexpr float clusterX = -0.75f;
        constexpr float clusterY = 0.8f;
        constexpr float clusterZ = 2.38f;

        // Upper Left short beam
        AddLightPart(shader, PoliceCarLightType::Headlight, FacePosition::Left, PoliceCarPartIndex::HeadLight_L, ShadowMapIndex::PL_Headlight_L,
            shortBeamColor,
            glm::vec3(clusterX, clusterY + 0.1f, clusterZ),
            glm::vec3(0.5f, 0.1f, 0.1f)
        );

        // Bottom left high beam
        AddLightPart(shader, PoliceCarLightType::HighBeam, FacePosition::Left, PoliceCarPartIndex::HighBeam_L, ShadowMapIndex::PL_Highbeam_L,
            highBeamColor,
            glm::vec3(clusterX + 0.125f, clusterY, clusterZ),
            glm::vec3(0.25f, 0.1f, 0.1f)
        );

        // Bottom left blinker
        AddPointLightPart(shader, PoliceCarLightType::Blinker, FacePosition::Left | FacePosition::Front, PoliceCarPartIndex::Blinker_FL, ShadowCubeIndex::Blinker_FL,
            blinkerColor,
            glm::vec3(clusterX - 0.125f, clusterY, clusterZ),
            glm::vec3(0.25f, 0.1f, 0.1f)
        );

        // Right Front Light Cluster
        // Upper right short beam
        AddLightPart(shader, PoliceCarLightType::Headlight, FacePosition::Right, PoliceCarPartIndex::HeadLight_R, ShadowMapIndex::PL_Headlight_R,
            shortBeamColor, 
            glm::vec3(-clusterX, clusterY + 0.1f, clusterZ),
            glm::vec3(0.5f, 0.1f, 0.1f)
        );

        // Bottom right high beam
        AddLightPart(shader, PoliceCarLightType::HighBeam, FacePosition::Right, PoliceCarPartIndex::HighBeam_R, ShadowMapIndex::PL_Highbeam_R,
            highBeamColor, 
            glm::vec3(-clusterX - 0.125f, clusterY, clusterZ),
            glm::vec3(0.25f, 0.1f, 0.1f)
        );

        // Bottom right blinker
        AddPointLightPart(shader, PoliceCarLightType::Blinker, FacePosition::Right | FacePosition::Front, PoliceCarPartIndex::Blinker_FR, ShadowCubeIndex::Blinker_FR,
            blinkerColor, 
            glm::vec3(-clusterX + 0.125f, clusterY, clusterZ),
            glm::vec3(0.25f, 0.1f, 0.1f)
        );

    /* --- STOP FRONT LIGHTS --- */

    /* --- START BACK LIGHTS --- */

        constexpr float rClusterX = -0.75f;
        constexpr float rClusterY = 0.8f;
        constexpr float rClusterZ = -2.38f;

        // Left side rear light cluster
        // LEFT SIDE AS IN LOOKING FROM THE FRONT OF THE CAR!
        // Top left brake light
        AddLightPart(shader, PoliceCarLightType::BrakeLight, FacePosition::Left, PoliceCarPartIndex::RearLight_L, ShadowMapIndex::PL_Rearlight_L,
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(rClusterX, rClusterY + 0.1f, rClusterZ),
            glm::vec3(0.5f, 0.1f, 0.1f)
        );

        // Bottom left reverse light
        AddLightPart(shader, PoliceCarLightType::ReverseLight, FacePosition::Left, PoliceCarPartIndex::ReverseLight_L, ShadowMapIndex::PL_Reverselight_L,
            glm::vec3(0.96f, 0.96f, 0.72f),
            glm::vec3(rClusterX + 0.125f, rClusterY, rClusterZ),
            glm::vec3(0.25f, 0.1f, 0.1f)
        );

        // Bottom left blinker light
        AddPointLightPart(shader, PoliceCarLightType::Blinker, FacePosition::Left | FacePosition::Behind, PoliceCarPartIndex::Blinker_BL, ShadowCubeIndex::Blinker_BL,
            glm::vec3(1.0f, 0.5f, 0.0f),
            glm::vec3(rClusterX - 0.125f, rClusterY, rClusterZ),
            glm::vec3(0.25f, 0.1f, 0.1f)
        );

        // Right side back light cluster
        // AS IN LOOKING FROM THE FRONT OF THE CAR!
        // Top right brake light
        AddLightPart(shader, PoliceCarLightType::BrakeLight, FacePosition::Right, PoliceCarPartIndex::RearLight_R, ShadowMapIndex::PL_Rearlight_R,
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-rClusterX, rClusterY + 0.1f, rClusterZ),
            glm::vec3(0.5f, 0.1f, 0.1f)
        );

        // Bottom right reverse light
        AddLightPart(shader, PoliceCarLightType::ReverseLight, FacePosition::Right, PoliceCarPartIndex::ReverseLight_R, ShadowMapIndex::PL_Reverselight_R,
            glm::vec3(0.96f, 0.96f, 0.72f),
            glm::vec3(-rClusterX - 0.125f, rClusterY, rClusterZ),
            glm::vec3(0.25f, 0.1f, 0.1f)
        );

        // Botom right blinker
        AddPointLightPart(shader, PoliceCarLightType::Blinker, FacePosition::Right | FacePosition::Behind, PoliceCarPartIndex::Blinker_BR, ShadowCubeIndex::Blinker_BR,
            glm::vec3(1.0f, 0.5f, 0.0f),
            glm::vec3(-rClusterX + 0.125f, rClusterY, rClusterZ),
            glm::vec3(0.25f, 0.1f, 0.1f)
        );

    /* --- STOP BACK LIGHTS --- */
}

void PoliceCar::AddBodyPart(const std::weak_ptr<Shader>& shader, PoliceCarMeshType type, PoliceCarPartIndex index, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos,.rotation = rot, .scale = scale }, color);

    PoliceCarPart part;
    part.meshData = std::make_unique<PoliceCarMesh>();
    part.meshData->mesh = std::move(mesh);
    part.meshData->type = type;
    part.meshData->offset = pos;

    carPartIndex[static_cast<size_t>(index)] = parts.size();
    parts.push_back(std::move(part));
}

void PoliceCar::AddLightPart(const std::weak_ptr<Shader>& shader, PoliceCarLightType lType, FacePosition side, PoliceCarPartIndex index, ShadowMapIndex shadowMapindex, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    PoliceCarPart part;
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);

    part.meshData = std::make_unique<PoliceCarMesh>();
    part.meshData->mesh = std::move(mesh);
    part.meshData->type = PoliceCarMeshType::Light;
    part.meshData->lightMeshIntensity = 1.0f;
    part.meshData->offset = pos;
    part.meshData->castShadow = false;

    part.lightData = std::make_unique<PoliceCarLight>();
    part.lightData->type = lType;
    part.lightData->offset = pos;
    part.lightData->position.facePosition = side;


    if (lType == PoliceCarLightType::Headlight || lType == PoliceCarLightType::BrakeLight)
    {
        SpotLight spot;
        spot.base.baseColor = color;
        spot.base.intensity = 5.0f;
        spot.direction = lType == PoliceCarLightType::Headlight ? glm::vec3(0, 0, 1) : glm::vec3(0, 0, -1); // Facing Forward
        part.lightData->spotLight = std::make_unique<SpotLight>(spot);
        part.isActivated = true;
        part.meshData->lightMeshIntensity = 1.0f;
        part.lightData->spotLight->shadowMapIdx = shadowMapindex;
    }
    else if (lType == PoliceCarLightType::HighBeam)
    {
        SpotLight spot;
        spot.base.baseColor = color;
        spot.base.intensity = 0.0f;
        spot.cutOff = glm::cos(glm::radians(45.0f));
        spot.outerCutOff = glm::cos(glm::radians(60.0f));
        spot.direction = glm::vec3(0, 0, 1);
        part.lightData->spotLight = std::make_unique<SpotLight>(spot);
        part.isActivated = false;
        part.lightData->spotLight->shadowMapIdx = shadowMapindex;
    }
    else if (lType == PoliceCarLightType::ReverseLight)
    {
        SpotLight spot;
        spot.base.baseColor = color;
        spot.base.intensity = 0.0f;
        spot.direction = glm::vec3(0, 0, -1);
        part.lightData->spotLight = std::make_unique<SpotLight>(spot);
        part.isActivated = false;
        part.lightData->spotLight->shadowMapIdx = shadowMapindex;
    }

    carPartIndex[static_cast<size_t>(index)] = parts.size();
    parts.push_back(std::move(part));
}

void PoliceCar::AddPointLightPart(const std::weak_ptr<Shader>& shader, PoliceCarLightType lType, FacePosition side, PoliceCarPartIndex index, ShadowCubeIndex shadowCubeIndex, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    PoliceCarPart part;
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);

    part.meshData = std::make_unique<PoliceCarMesh>();
    part.meshData->mesh = std::move(mesh);
    part.meshData->type = PoliceCarMeshType::Light;
    part.meshData->lightMeshIntensity = 1.0f;
    part.meshData->offset = pos;
    part.meshData->castShadow = false;

    part.lightData = std::make_unique<PoliceCarLight>();
    part.lightData->type = lType;
    part.lightData->offset = pos;
    part.lightData->position.facePosition = side;

    if (lType == PoliceCarLightType::LightBar)
    {
        PointLight point;
        point.base.baseColor = color;
        point.base.intensity = 15.0f;
        part.lightData->offset.y += 1.5f;

        // Nudge it outward slightly based on which side it's on
        if (HasFlag(side, FacePosition::Left))
        {
            part.lightData->offset.x += 0.3f;
        }
        else if (HasFlag(side, FacePosition::Right)) part.lightData->offset.x -= 0.3f;
        part.lightData->pointLight = std::make_unique<PointLight>(point);
        part.lightData->flashing = true;
        part.lightData->pulseSpeed = 2.0f;
    }
    else if (lType == PoliceCarLightType::Blinker)
    {
        PointLight point;
        point.base.baseColor = color;
        point.base.intensity = 0.0f;
        float nudge = HasFlag(side, FacePosition::Behind) ? -0.2f : 0.2f;
        //part.lightData->offset.z += nudge;
        part.lightData->pointLight = std::make_unique<PointLight>(point);
        part.isActivated = false;
        part.lightData->flashing = true;
        part.lightData->pulseSpeed = 3.0f;
    }

    part.lightData->pointLight->shadowCubeIdx = shadowCubeIndex;

    carPartIndex[static_cast<size_t>(index)] = parts.size();
    parts.push_back(std::move(part));
}

void PoliceCar::AddWheel(const std::weak_ptr<Shader>& shader, PoliceCarPartIndex index, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    PoliceCarPart part;

    Transform t;
    t.position = pos;
    t.scale = scale;
    t.rotation = glm::vec3(0, 0, glm::radians(90.0f)) + rot;

    auto mesh = std::make_unique<CylinderMesh>(shader, 0.5f, 1.0f, 32, t, color);

    part.meshData = std::make_unique<PoliceCarMesh>();
    part.meshData->mesh = std::move(mesh);
    part.meshData->type = PoliceCarMeshType::Wheel; 
    part.meshData->offset = pos;

    carPartIndex[static_cast<size_t>(index)] = parts.size();
    parts.push_back(std::move(part));
}

void PoliceCar::Bake(const std::weak_ptr<Shader>& shader)
{
    std::vector<float> allVertices;
    std::vector<unsigned int> allIndices;

    for (size_t i = 0; i < parts.size(); ++i) {
        auto& part = parts[i];
        if (!part.meshData) continue;

        // Skip dynamic parts (Wheels, Lights, etc.)
        if (part.meshData->type == PoliceCarMeshType::Wheel || part.meshData->type == PoliceCarMeshType::Light)
        {
            dynamicPartIndices.push_back(i);
            continue;
        }

        auto& mesh = part.meshData->mesh;

        // GetVertexData now returns the 14-float format from the GPU
        std::vector<float> verts = mesh->GetVertexData();
        std::vector<unsigned int> inds = mesh->GetIndexData();

        glm::mat4 localMat = mesh->GetTransform().GetModelMatrix();
        glm::mat3 normMat = glm::mat3(glm::transpose(glm::inverse(localMat)));

        unsigned int vertOffset = static_cast<unsigned int>(allVertices.size() / STRIDE);

        // Process Vertices (Step by 14)
        for (size_t v = 0; v < verts.size(); v += STRIDE) {
            // 1. Position (0, 1, 2) - Transform by model matrix
            glm::vec4 pos = localMat * glm::vec4(verts[v], verts[v + 1], verts[v + 2], 1.0f);

            // 2. Normal (3, 4, 5) - Transform by normal matrix
            glm::vec3 norm = glm::normalize(normMat * glm::vec3(verts[v + 3], verts[v + 4], verts[v + 5]));

            // Push transformed data
            allVertices.push_back(pos.x);
            allVertices.push_back(pos.y);
            allVertices.push_back(pos.z);

            allVertices.push_back(norm.x);
            allVertices.push_back(norm.y);
            allVertices.push_back(norm.z);

            // 3. Copy UVs, Tangents, and Color (Indices 6 through 13)
            // 6-7: UVs
            // 8-10: Tangents
            // 11-13: Color
            for (int k = 6; k < 14; ++k) {
                allVertices.push_back(verts[v + k]);
            }
        }

        // Process Indices
        for (auto idx : inds) {
            allIndices.push_back(idx + vertOffset);
        }
    }

    // Create the final baked mesh
    // Note: We use the constructor that accepts the raw data.
    // Ensure your Mesh constructor knows that if input is already 14-stride, 
    // it shouldn't try to "expand" it again.
    this->bakedBodyMesh = std::make_unique<Mesh>(
        allVertices.data(), allVertices.size(),
        shader, Transform(), glm::vec3(1.0f), // meshColor here is ignored by shader now
        allIndices.data(), allIndices.size()
    );
}

void PoliceCar::HandlePointLight(PoliceCarPart& part) const
{
    if (!part.lightData->pointLight)
    {
        return;
    }
    auto& lightData = part.lightData;
    auto& pointLight = lightData->pointLight;

    glm::mat4 carRotation = glm::rotate(glm::mat4(1.0f), transform.rotation.y, glm::vec3(0, 1, 0));
    glm::vec4 rotatedOffset = carRotation * glm::vec4(lightData->offset, 1.0f);

    pointLight->base.position = transform.position + glm::vec3(rotatedOffset);

    if (lightData->flashing)
    {
        // Classic alternating light bar pattern
        // Use sirenTimer * pulseSpeed to control rhythm
        float t = static_cast<float>(sirenTimer) * lightData->pulseSpeed;

        bool on = false;
        if (lightData->type == PoliceCarLightType::LightBar)
        {
            // Left/right lights alternate using the X coordinate sign
            if (HasFlag(lightData->position.facePosition, FacePosition::Left))
            {
                on = fmod(t, 2.0f) < 1.0f; // first second: on, second second: off
            }
            else if (HasFlag(lightData->position.facePosition, FacePosition::Right))
            {
                on = fmod(t, 2.0f) >= 1.0f; // opposite phase
            }

            on = on && isLightBarOn;
            part.lightData->pointLight->base.intensity = on ? 15.0f : 0.0f;
        }
        else if(lightData->type ==  PoliceCarLightType::Blinker)
        {
            bool sideMatches = false;
            if (blinkerSide == BlinkerSide::HazardLights) 
            {
                sideMatches = true;
            }
            else if (blinkerSide == BlinkerSide::LeftSide && HasFlag(lightData->position.facePosition, FacePosition::Left)) 
            {
                sideMatches = true;
            }
            else if (blinkerSide == BlinkerSide::RightSide && HasFlag(lightData->position.facePosition, FacePosition::Right)) 
            {
                sideMatches = true;
            }

            if (sideMatches) 
            {
                float t = static_cast<float>(blinkerTimer) * lightData->pulseSpeed;
                on = fmod(t, 2.0f) < 1.0f;
            }
            else 
            {
                on = false;
            }

            part.lightData->pointLight->base.intensity = on ? 1.0f : 0.0f;
        }
        else
        {
            on = true;
        }

        part.isActivated = on;
        pointLight->base.currentColor = on ? pointLight->base.baseColor : glm::vec3(0.0f);
    }
    else
    {
        // Constant lights (headlights, etc.)
        part.isActivated = true;
        pointLight->base.currentColor = pointLight->base.baseColor;
    }
}

void PoliceCar::HandleSpotLight(PoliceCarPart& part) const
{
    if (!part.lightData->spotLight)
    {
        return;
    }

    auto& lightData = part.lightData;
    auto& spotLight = lightData->spotLight;

    glm::mat4 carRotation = glm::rotate(glm::mat4(1.0f), transform.rotation.y, glm::vec3(0, 1, 0));

    // 2. Transform the Position
    // We treat the offset as a vec4(pos, 1.0) to multiply by the matrix
    glm::vec3 worldLightPos = glm::vec3(carRotation * glm::vec4(lightData->offset, 1.0f));
    spotLight->base.position = transform.position + worldLightPos;

    // 3. Transform the Direction
    // Directions are vectors, so we use vec4(dir, 0.0). This ignores translation.
    glm::vec3 worldLightDir = glm::vec3(carRotation * glm::vec4(lightData->offset, 0.0f));
    spotLight->direction = glm::normalize(worldLightDir);

    spotLight->base.currentColor = spotLight->base.baseColor;

    if (lightData->flashing)
    {
        // Classic alternating light bar pattern
        // Use sirenTimer * pulseSpeed to control rhythm
        float t = static_cast<float>(sirenTimer) * lightData->pulseSpeed;

        bool on = false;
        if (lightData->type == PoliceCarLightType::LightBar)
        {
            // Left/right lights alternate using the X coordinate sign
            if (HasFlag(lightData->position.facePosition, FacePosition::Left))
            {
                on = fmod(t, 2.0f) < 1.0f; // first second: on, second second: off
            }
            else if (HasFlag(lightData->position.facePosition, FacePosition::Right))
            {
                on = fmod(t, 2.0f) >= 1.0f; // opposite phase
            }
        }
        else
        {
            on = true;
        }

        part.isActivated = on;
        spotLight->base.currentColor = on ? spotLight->base.baseColor : glm::vec3(0.0f);
    }
    else if (lightData->type != PoliceCarLightType::HighBeam 
        && lightData->type != PoliceCarLightType::ReverseLight
        && lightData->type != PoliceCarLightType::BrakeLight
        && lightData->type != PoliceCarLightType::Headlight)
    {
        // Constant lights (headlights, etc.)
        part.isActivated = true;
    }
}