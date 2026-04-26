#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

#include "PoliceCarActive.h"
#include "CubeMesh.h"
#include "MeshCache.h"
#include "CivilianCarActive.h"

PoliceCarActive::PoliceCarActive(const std::weak_ptr<Shader>& shader, const Transform& transform) :
	PoliceCarModel(shader, transform) 
{
    InitParts(shader);
    Bake(shader);
}

void PoliceCarActive::Update(double deltaTime)
{
    float dt = static_cast<float>(deltaTime);

    UpdatePhysics(dt);

    sirenTimer += deltaTime;
    blinkerTimer += deltaTime;

    for (auto& part : data->parts)
    {
        if (part.lightData)
        {
            if (part.lightData->pointLight)
            {
                HandlePointLight(part);
            }
            else if (part.lightData->spotLight)
            {
                HandleSpotLight(part);
            }
        }

        if (part.meshData)
        {
            switch (part.meshData->type)
            {
                case PoliceCarMeshType::Wheel:
                    UpdateWheelTransform(part);
                    break;
                case PoliceCarMeshType::Steering:
                    UpdateSteeringTransform(part);
                    break;
                default:
                    break;
            }
        }
    }
}

void PoliceCarActive::Draw(std::shared_ptr<Shader> customShader) const
{
    glm::mat4 baseMatrix = transform.GetModelMatrix();

    // LEAN NEEDS TO BE LOCAL!
    glm::mat4 carMatrix = baseMatrix * CalculateLeanMatrix();

    auto activeShader = customShader ? customShader : data->bakedBodyMesh->GetShader().lock();
    if (!activeShader)
    {
        return;
    }

    activeShader->UseShader();

    glUniformMatrix4fv(activeShader->GetUniformLocation("modelMatrix"), 1, GL_FALSE, &carMatrix[0][0]);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(carMatrix)));
    glUniformMatrix3fv(activeShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

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
        auto& part = data->parts[idx];
        if (!part.meshData)
        {
            continue;
        }

        if (customShader && !part.meshData->castShadow)
        {
            continue;
        }

        auto& mesh = part.meshData->mesh;

        activeShader->UseShader();

        bool isLight = (part.meshData->type == PoliceCarMeshType::Light);
        glUniform1i(activeShader->GetUniformLocation("isLightMesh"), isLight);

        if (isLight)
        {
            float intensity = part.isActivated ? part.meshData->lightMeshIntensity : INACTIVE_LIGHT_MESH_INTENSITY;
            glUniform1f(activeShader->GetUniformLocation("lightMeshIntensity"), intensity);
        }

        glm::mat4 partLocalMatrix;
        if (part.meshData->type == PoliceCarMeshType::Wheel)
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

void PoliceCarActive::ProcessKeyboard(Direction direction, double deltaTime)
{
    float dt = static_cast<float>(deltaTime);

    if (direction == Direction::Forward)
    {
        speed += ACCELERATION * dt;
    }
    else if (direction == Direction::Backward)
    {
        speed -= ACCELERATION * dt;
    }

    flags.isBraking = (speed > MOVING_THRESHOLD) && direction == Direction::Backward;
    flags.isReversing = (speed < -MOVING_THRESHOLD);

    SetLightState(PoliceCarPartIndex::ReverseLight_L, flags.isReversing, REVERSE_LIGHT_INTENSITY);
    SetLightState(PoliceCarPartIndex::ReverseLight_R, flags.isReversing, REVERSE_LIGHT_INTENSITY);

    float targetSteer = 0.0f;
    if (std::abs(speed) > MOVING_THRESHOLD)
    {
        if (direction == Direction::Left)
        {
            targetSteer = TURN_ANGLE;
        }
        else if (direction == Direction::Right)
        {
            targetSteer = -TURN_ANGLE;
        }
    }
    steerAngle = glm::mix(steerAngle, targetSteer, dt * TURN_ANGLE_INTERPOLATOR);
}

void PoliceCarActive::ToggleHighBeams()
{
    flags.highBeamsOn = !flags.highBeamsOn;
    SetLightState(PoliceCarPartIndex::HighBeam_L, flags.highBeamsOn, HIGHBEAM_INTENSITY);
    SetLightState(PoliceCarPartIndex::HighBeam_R, flags.highBeamsOn, HIGHBEAM_INTENSITY);
}

void PoliceCarActive::ToggleShortBeams()
{
    flags.headlightsOn = !flags.headlightsOn;
    SetLightState(PoliceCarPartIndex::HeadLight_L, flags.headlightsOn, SHORTBEAM_INTENSITY);
    SetLightState(PoliceCarPartIndex::HeadLight_R, flags.headlightsOn, SHORTBEAM_INTENSITY);
}

void PoliceCarActive::SetBlinkerState(BlinkerSide side)
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

void PoliceCarActive::ToggleLightBar()
{
    flags.isLightBarOn = !flags.isLightBarOn;
    sirenTimer = 0.0;
}

void PoliceCarActive::ToggleRearLights() { flags.brakeLightsOn = !flags.brakeLightsOn; }

std::vector<PoliceCarLight*> PoliceCarActive::GetAllLights()
{
    std::vector<PoliceCarLight*> lights;
    lights.reserve(data->parts.size());
    for (auto& part : data->parts)
    {
        if (part.lightData) 
        {
            lights.push_back(part.lightData.get());
        }
    }
    return lights;
}

glm::vec3 PoliceCarActive::GetForward() const
{
    return glm::vec3(sin(glm::radians(currentAngle)), 0.0f, cos(glm::radians(currentAngle)));
}

glm::vec3 PoliceCarActive::GetPosition() const { return transform.position; }

glm::vec3 PoliceCarActive::GetRotation() const { return transform.rotation; }

glm::vec3 PoliceCarActive::GetWorldPosOfLight(PoliceCarPartIndex partIndex) const
{
    const PoliceCarPart* part = GetPart(partIndex);
    if (!part)
    {
        return transform.position;
    }

    glm::vec3 localOffset = (part->lightData) ? part->lightData->offset : part->meshData->offset;

    return TransformOffsetToWorld(localOffset);
}

float PoliceCarActive::GetSpotlightOuterAngle(PoliceCarPartIndex partIndex) const
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

void PoliceCarActive::AddSpotLightPart(const std::weak_ptr<Shader>& shader, PoliceCarLightType lType, FacePosition side, PoliceCarPartIndex index, ShadowMapIndex shadowMapindex, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    PoliceCarPart part = CreateBaseLightPart(shader, lType, side, color, pos, scale, rot);

    SpotLight spot;
    spot.base.baseColor = color;
    spot.shadowMapIdx = shadowMapindex;
    spot.base.intensity = 0.0f;
    spot.direction = glm::vec3(0, 0, 1);

    switch (lType)
    {
        case PoliceCarLightType::Headlight:
        {
            part.isActivated = true;
            break;
        }
        case PoliceCarLightType::BrakeLight:
        {
            spot.direction = glm::vec3(0, 0, -1);
            part.isActivated = true;
            break;
        }
        case PoliceCarLightType::HighBeam:
        {
            spot.cutOff = glm::cos(glm::radians(HIGHBEAM_INNER_CUTOFF));
            spot.outerCutOff = glm::cos(glm::radians(HIGHBEAM_OUTER_CUFOFF));
            break;
        }
        case PoliceCarLightType::ReverseLight:
        {
            spot.direction = glm::vec3(0, 0, -1);
            break;
        }
        default: break;
    }

    part.lightData->spotLight = std::make_unique<SpotLight>(spot);

    data->carPartIndex[static_cast<size_t>(index)] = data->parts.size();
    data->parts.push_back(std::move(part));
}

void PoliceCarActive::AddPointLightPart(const std::weak_ptr<Shader>& shader, PoliceCarLightType lType, FacePosition side, PoliceCarPartIndex index, ShadowCubeIndex shadowCubeIndex, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    PoliceCarPart part = CreateBaseLightPart(shader, lType, side, color, pos, scale, rot);
    part.lightData->flashing = true;
    PointLight point;
    point.base.baseColor = color;
    point.shadowCubeIdx = shadowCubeIndex;
    point.base.intensity = 0.0f;

    switch (lType)
    {
        case PoliceCarLightType::LightBar:
        {
            constexpr float nudgeX = 0.3f;
            part.lightData->offset.y += 1.5f;
            if (HasFlag(side, FacePosition::Left))
            {
                part.lightData->offset.x += nudgeX;
            }
            else if (HasFlag(side, FacePosition::Right))
            {
                part.lightData->offset.x -= nudgeX;
            }
            part.lightData->pulseSpeed = LIGHTBAR_PULSE_SPEED;
            break;
        }
        case PoliceCarLightType::Blinker:
        {
            constexpr float nudgeZ = 0.1f;
            if (HasFlag(side, FacePosition::Front))
            {
                part.lightData->offset.z += nudgeZ;
            }
            else if (HasFlag(side, FacePosition::Behind))
            {
                part.lightData->offset.z -= nudgeZ;
            }
            part.lightData->pulseSpeed = BLINKER_PULSE_SPEED;
            break;
        }
        default: break;
    }

    part.lightData->pointLight = std::make_unique<PointLight>(point);

    data->carPartIndex[static_cast<size_t>(index)] = data->parts.size();
    data->parts.push_back(std::move(part));
}

std::unique_ptr<Mesh> PoliceCarActive::PerformBakePass(const std::weak_ptr<Shader>& shader, BakePassType passType)
{
    std::vector<float> allVertices;
    std::vector<unsigned int> allIndices;

    std::vector<MeshCache> batches;
    batches.reserve(data->parts.size());

    size_t totalVertFloats = 0;
    size_t totalIndices = 0;

    dynamicPartIndices.clear();

    for (size_t i = 0; i < data->parts.size(); ++i)
    {
        auto& part = data->parts[i];
        if (!part.meshData)
        {
            continue;
        }

        if (part.meshData->type == PoliceCarMeshType::Wheel ||
            part.meshData->type == PoliceCarMeshType::Light ||
            part.meshData->type == PoliceCarMeshType::Steering)
        {
            dynamicPartIndices.push_back(i);
            continue;
        }

        bool isWindow = (part.meshData->type == PoliceCarMeshType::Window);

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

glm::mat4 PoliceCarActive::CalculateWheelMatrix(const PoliceCarPart& part) const
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, part.meshData->offset);

    // Steering (only for front wheels)
    if (part.meshData->offset.z > 0)
    {
        // If needed, rotate Y (yaw)
        m = glm::rotate(m, part.meshData->mesh->GetTransform().rotation.y, glm::vec3(0, 1, 0));
    }

    // Constant Wheel orientation + Rolling rotation
    // 90 DEGREES ROTATION! THE AXES CHANGE!
    m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 0, 1)); // (Rotate Z; roll, so that they're vertical)
    m = glm::rotate(m, part.meshData->mesh->GetTransform().rotation.x, glm::vec3(0, 1, 0)); // ROTATE AROUND Y WITH X VALUE BECAUSE THE WHEEL IS ROTATE 90 DEGREES!!!!
    m = glm::scale(m, part.meshData->mesh->GetTransform().scale);
    return m;
}

glm::mat4 PoliceCarActive::CalculateLeanMatrix() const
{
    glm::mat4 lean = glm::mat4(1.0f);
    // Apply pitch first (X-axis; rotate front-back) then roll (Z-axis; rotate left right)
    lean = glm::rotate(lean, glm::radians(visualPitch), glm::vec3(1, 0, 0));
    lean = glm::rotate(lean, glm::radians(visualRoll), glm::vec3(0, 0, 1));
    return lean;
}

void PoliceCarActive::UpdatePhysics(double deltaTime)
{
    float dt = static_cast<float>(deltaTime);

    // Apply Friction / Acceleration
    if (speed > 0)
    {
        speed = std::max(0.0f, speed - DECELERATION * dt);
    }
    else if (speed < 0)
    {
        speed = std::min(0.0f, speed + DECELERATION * dt);
    }

    if (glm::abs(speed) < STOPPED_THRESHOLD)
    {
        speed = 0.0f;
    }

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

    float targetPitchDeg = (speed > MOVING_THRESHOLD && flags.isBraking) ? 2.0f : 0.0f;
    visualPitch = glm::mix(visualPitch, targetPitchDeg, dt * 5.0f);

    wheelRotation += speed * dt * WHEEL_ROTATION_FACTOR;
}

void PoliceCarActive::UpdateWheelTransform(PoliceCarPart& part) const
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

void PoliceCarActive::UpdateSteeringTransform(PoliceCarPart& part) const
{
    Transform t = part.meshData->mesh->GetTransform();
    t.rotation.y = glm::radians(glm::clamp(-steerAngle, -120.0f, 120.0f));
    part.meshData->mesh->SetTransform(t);
}

PoliceCarPart PoliceCarActive::CreateBaseLightPart(const std::weak_ptr<Shader>& shader, PoliceCarLightType lType, FacePosition side, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    PoliceCarPart part;
    part.isActivated = false;

    // Setup Mesh
    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ .position = pos, .rotation = rot, .scale = scale }, color);
    part.meshData = std::make_unique<PoliceCarMesh>();
    part.meshData->mesh = std::move(mesh);
    part.meshData->type = PoliceCarMeshType::Light;
    part.meshData->lightMeshIntensity = INACTIVE_LIGHT_MESH_INTENSITY;
    part.meshData->offset = pos;
    part.meshData->castShadow = false;

    // Setup Light Metadata
    part.lightData = std::make_unique<PoliceCarLight>();
    part.lightData->type = lType;
    part.lightData->offset = pos;
    part.lightData->position.facePosition = side;

    return part;
}

void PoliceCarActive::SetLightState(PoliceCarPartIndex index, bool active, float intensity)
{
    PoliceCarPart* part = GetPart(index);
    if (!part)
    {
        return;
    }

    part->isActivated = active;

    if (part->meshData)
    {
        part->meshData->lightMeshIntensity = active ? ACTIVE_LIGHT_MESH_INTENSITY : 0.0f;
    }

    if (part->lightData)
    {
        float finalIntensity = active ? intensity : 0.0f;

        if (part->lightData->spotLight)
        {
            part->lightData->spotLight->base.intensity = finalIntensity;
        }
        else if (part->lightData->pointLight)
        {
            part->lightData->pointLight->base.intensity = finalIntensity;
        }
    }
}

glm::vec3 PoliceCarActive::TransformOffsetToWorld(const glm::vec3& localOffset) const
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), transform.rotation.y, glm::vec3(0, 1, 0));
    glm::vec3 worldOffset = glm::vec3(rotation * glm::vec4(localOffset, 1.0f));
    return transform.position + worldOffset;
}

void PoliceCarActive::HandlePointLight(PoliceCarPart& part) const
{
    if (part.lightData && part.lightData->pointLight)
    {
        UpdateBaseProperties(part.lightData->pointLight->base, part);
    }
}

void PoliceCarActive::HandleSpotLight(PoliceCarPart& part) const
{
    if (part.lightData && part.lightData->spotLight)
    {
        UpdateBaseProperties(part.lightData->spotLight->base, part);

        // Get the car's rotation matrix
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), transform.rotation.y, glm::vec3(0, 1, 0));

        // Define the local direction (Front lights face +Z, Back face -Z)
        glm::vec3 localDir = glm::vec3(0.0f, 0.0f, 1.0f);
        if (part.lightData->type == PoliceCarLightType::BrakeLight ||
            part.lightData->type == PoliceCarLightType::ReverseLight)
        {
            localDir = glm::vec3(0.0f, 0.0f, -1.0f);
        }

        // Transform direction into world space
        part.lightData->spotLight->direction = glm::vec3(rotation * glm::vec4(localDir, 0.0f));
    }
}

void PoliceCarActive::UpdateBaseProperties(LightProperties& base, PoliceCarPart& part) const
{
    float intensity = 0.0f;
    bool on = CalculateLightState(part, intensity);

    base.position = TransformOffsetToWorld(part.lightData->offset);
    base.intensity = intensity;
    base.currentColor = on ? base.baseColor : glm::vec3(0.0f);

    if (part.meshData)
    {
        if (on) 
        {
            // If the light is on, scale the mesh glow based on how bright the light is.
            // For example: 8.0 intensity (Brake) -> 1.0 glow
            //              2.0 intensity (Tail)  -> 0.5 glow
            if (part.lightData->type == PoliceCarLightType::BrakeLight) 
            {
                part.meshData->lightMeshIntensity = (intensity > BRAKE_LIGHT_IDLE_INTENSITY) ? 1.0f : 0.5f;
            }
            else 
            {
                part.meshData->lightMeshIntensity = ACTIVE_LIGHT_MESH_INTENSITY;
            }
        }
        else 
        {
            part.meshData->lightMeshIntensity = INACTIVE_LIGHT_MESH_INTENSITY;
        }
    }

    part.isActivated = on;
}

bool PoliceCarActive::CalculateLightState(const PoliceCarPart& part, float& outIntensity) const
{
    auto& lightData = part.lightData;

    // Handle non-flashing lights first
    if (!lightData->flashing)
    {
        switch (lightData->type)
        {
            case PoliceCarLightType::BrakeLight:
                if (!flags.brakeLightsOn)
                {
                    outIntensity = 0.0f;
                    return false;
                }
                outIntensity = (flags.isBraking || speed == 0.0f) ? BRAKE_LIGHT_USE_INTENSITY : BRAKE_LIGHT_IDLE_INTENSITY;
                return true;

            case PoliceCarLightType::Headlight:
                outIntensity = flags.headlightsOn ? SHORTBEAM_INTENSITY : 0.0f;
                return flags.headlightsOn;
            case PoliceCarLightType::ReverseLight:
                outIntensity = flags.isReversing ? REVERSE_LIGHT_INTENSITY : 0.0f;
                return flags.isReversing;
            case PoliceCarLightType::HighBeam:
                outIntensity = flags.highBeamsOn ? HIGHBEAM_INTENSITY : 0.0f;
                return flags.highBeamsOn;
            default:
                return true;
        }
    }

    bool on = false;
    float defaultIntensity = 1.0f;

    switch (lightData->type)
    {
        case PoliceCarLightType::LightBar:
        {
            float t = static_cast<float>(sirenTimer) * lightData->pulseSpeed;
            bool isLeft = HasFlag(lightData->position.facePosition, FacePosition::Left);
            on = isLeft ? (fmod(t, 2.0f) < 1.0f) : (fmod(t, 2.0f) >= 1.0f);
            on = on && flags.isLightBarOn;
            defaultIntensity = LIGHTBAR_INTENSITY;
            break;
        }
        case PoliceCarLightType::Blinker:
        {
            bool sideMatches = (blinkerSide == BlinkerSide::HazardLights) ||
                (blinkerSide == BlinkerSide::LeftSide && HasFlag(lightData->position.facePosition, FacePosition::Left)) ||
                (blinkerSide == BlinkerSide::RightSide && HasFlag(lightData->position.facePosition, FacePosition::Right));

            if (sideMatches)
            {
                float t = static_cast<float>(blinkerTimer) * lightData->pulseSpeed;
                on = fmod(t, 2.0f) < 1.0f;
            }
            defaultIntensity = BLINKER_INTENSITY;
            break;
        }
        default:
            on = true;
            break;
    }

    outIntensity = on ? defaultIntensity : 0.0f;
    return on;
}