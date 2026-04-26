#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "Application.h"

#include "Shader.h"
#include "Mesh.h"
#include "PoliceCar.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "CubeMesh.h"
#include "PoliceCarFactory.h"

Application::~Application() { Shutdown(); }

bool Application::Init()
{
    // Initalise GLFW.
    if (!glfwInit()) 
    {
        std::cerr << "{Application::Init} Failed to init GLFW" << std::endl;
        return false;
    }

    // Create window.
    window = glfwCreateWindow(1920, 1080, "OpenGL", NULL, NULL);
    if (!window) 
    {
        std::cout << "{Application::Init} Failed to create window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Make the window the context.
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    // Load GLAD.
    if (!gladLoadGL()) 
    {
        std::cout << "{Application::Init} Failed to init GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(
        [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
            const GLchar* message, const void* userParam)
        {
            std::cerr << "[OpenGL DEBUG] " << message << std::endl;
        }, nullptr
    );

    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << std::endl;
    const GLubyte* renderer = glGetString(GL_RENDERER);
    std::cout << "Renderer: " << renderer << std::endl;

    auto phong = std::make_shared<Shader>("phong.vert", "phong.frag");
    phong->SetName("PHONG");
    shaders[static_cast<size_t>(ShaderType::Phong)] = phong;

    auto depth = std::make_shared<Shader>("shadow_map.vert", "shadow_map.frag");
    depth->SetName("SHADOW_MAP");
    shaders[static_cast<size_t>(ShaderType::ShadowMap)] = std::move(depth);

    auto cube = std::make_shared<Shader>("shadow_cube.vert", "shadow_cube.frag", "shadow_cube.geom");
    cube->SetName("SHADOW_CUBE");
    shaders[static_cast<size_t>(ShaderType::ShadowCube)] = std::move(cube);

    auto xray = std::make_shared<Shader>("xray.vert", "xray.frag");
    xray->SetName("XRAY");
    shaders[static_cast<size_t>(ShaderType::Xray)] = std::move(xray);

    /*
        Shadow maps are influenced by
            Dimensions of the shadow map (if it's too high and the light is too small, also bad)
            Bias (smaller -> more moire, less peter panning, bigger -> opposite)
            PFC (mean value around pixel) (more -> softer shadows, can hide moire, more expensive)
            Texel size constant for PFC depth (more -> less moire); float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * (texelSize * 2.0)).r;
    */
    auto headlightL = std::make_unique<ShadowMap>(1024, 1024, 50.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    auto headlightR = std::make_unique<ShadowMap>(1024, 1024, 50.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    shadowMaps[static_cast<size_t>(ShadowMapIndex::PL_Headlight_L)] = ShadowMapEntry{ 
        .shadowMap = std::move(headlightL), 
        .forward = 1,
        .partIndex = PoliceCarPartIndex::HeadLight_L 
    };
    shadowMaps[static_cast<size_t>(ShadowMapIndex::PL_Headlight_R)] = ShadowMapEntry{
        .shadowMap = std::move(headlightR),
        .forward = 1,
        .partIndex = PoliceCarPartIndex::HeadLight_R
    };

    auto rearL = std::make_unique<ShadowMap>(1024, 1024, 20.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    auto rearR = std::make_unique<ShadowMap>(1024, 1024, 20.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    shadowMaps[static_cast<size_t>(ShadowMapIndex::PL_Rearlight_L)] = ShadowMapEntry{
        .shadowMap = std::move(rearL),
        .forward = -1,
        .partIndex = PoliceCarPartIndex::RearLight_L
    };
    shadowMaps[static_cast<size_t>(ShadowMapIndex::PL_Rearlight_R)] = ShadowMapEntry{
        .shadowMap = std::move(rearR),
        .forward = -1,
        .partIndex = PoliceCarPartIndex::RearLight_R
    };

    auto reverseL = std::make_unique<ShadowMap>(1024, 1024, 50.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    auto reverseR = std::make_unique<ShadowMap>(1024, 1024, 50.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    shadowMaps[static_cast<size_t>(ShadowMapIndex::PL_Reverselight_L)] = ShadowMapEntry{
        .shadowMap = std::move(reverseL),
        .forward = -1,
        .partIndex = PoliceCarPartIndex::ReverseLight_L
    };
    shadowMaps[static_cast<size_t>(ShadowMapIndex::PL_Reverselight_R)] = ShadowMapEntry{
        .shadowMap = std::move(reverseR),
        .forward = -1,
        .partIndex = PoliceCarPartIndex::ReverseLight_R
    };

    auto highBeamL = std::make_unique<ShadowMap>(2048, 2048, 100.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    auto highBeamR = std::make_unique<ShadowMap>(2048, 2048, 100.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    shadowMaps[static_cast<size_t>(ShadowMapIndex::PL_Highbeam_L)] = ShadowMapEntry{
        .shadowMap = std::move(highBeamL),
        .forward = 1,
        .partIndex = PoliceCarPartIndex::HighBeam_L
    };
    shadowMaps[static_cast<size_t>(ShadowMapIndex::PL_Highbeam_R)] = ShadowMapEntry{
        .shadowMap = std::move(highBeamR),
        .forward = 1,
        .partIndex = PoliceCarPartIndex::HighBeam_R
    };

    auto lightBarL = std::make_unique<ShadowCube>(1024, 1024);
    auto lightBarR = std::make_unique<ShadowCube>(1024, 1024);

    shadowCubes[static_cast<size_t>(ShadowCubeIndex::Lightbar_L)] = ShadowCubeEntry{
        .shadowCube = std::move(lightBarL),
        .partIndex = PoliceCarPartIndex::LightBar_L
    };

    shadowCubes[static_cast<size_t>(ShadowCubeIndex::Lightbar_R)] = ShadowCubeEntry{
        .shadowCube = std::move(lightBarR),
        .partIndex = PoliceCarPartIndex::LightBar_R
    };

    auto blinkerFrontLeft = std::make_unique<ShadowCube>(256, 256);
    auto blinkerFrontRight = std::make_unique<ShadowCube>(256, 256);

    shadowCubes[static_cast<size_t>(ShadowCubeIndex::Blinker_FL)] = ShadowCubeEntry{
        .shadowCube = std::move(blinkerFrontLeft),
        .partIndex = PoliceCarPartIndex::Blinker_FL
    };

    shadowCubes[static_cast<size_t>(ShadowCubeIndex::Blinker_FR)] = ShadowCubeEntry{
        .shadowCube = std::move(blinkerFrontRight),
        .partIndex = PoliceCarPartIndex::Blinker_FR
    };

    auto blinkerRearLeft = std::make_unique<ShadowCube>(256, 256);
    auto blinkerRearRight = std::make_unique<ShadowCube>(256, 256);

    shadowCubes[static_cast<size_t>(ShadowCubeIndex::Blinker_BL)] = ShadowCubeEntry{
        .shadowCube = std::move(blinkerRearLeft),
        .partIndex = PoliceCarPartIndex::Blinker_BL
    };

    shadowCubes[static_cast<size_t>(ShadowCubeIndex::Blinker_BR)] = ShadowCubeEntry{
        .shadowCube = std::move(blinkerRearRight),
        .partIndex = PoliceCarPartIndex::Blinker_BR
    };

    auto CV_headlightL = std::make_unique<ShadowMap>(1024, 1024, 50.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    auto CV_headlightR = std::make_unique<ShadowMap>(1024, 1024, 50.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    shadowMaps[static_cast<size_t>(ShadowMapIndex::CV_Headlight_L)] = ShadowMapEntry{
        .shadowMap = std::move(CV_headlightL),
        .forward = 1,
        .CV_partIndex = CivilianCarPartIndex::HeadLight_L
    };
    shadowMaps[static_cast<size_t>(ShadowMapIndex::CV_Headlight_R)] = ShadowMapEntry{
        .shadowMap = std::move(CV_headlightR),
        .forward = 1,
        .CV_partIndex = CivilianCarPartIndex::HeadLight_R
    };

    auto CV_rearL = std::make_unique<ShadowMap>(1024, 1024, 20.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    auto CV_rearR = std::make_unique<ShadowMap>(1024, 1024, 20.0f, 1.0f, 50.0f, 0.0f, ShadowType::Spot);
    shadowMaps[static_cast<size_t>(ShadowMapIndex::CV_Rearlight_L)] = ShadowMapEntry{
        .shadowMap = std::move(CV_rearL),
        .forward = -1,
        .CV_partIndex = CivilianCarPartIndex::RearLight_L
    };
    shadowMaps[static_cast<size_t>(ShadowMapIndex::CV_Rearlight_R)] = ShadowMapEntry{
        .shadowMap = std::move(CV_rearR),
        .forward = -1,
        .CV_partIndex = CivilianCarPartIndex::RearLight_R
    };

    moon = DirLight{
        .name = "moonLight",
        //.direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        .direction = glm::vec3(-0.5f, -0.7f, -0.3f),
        .color = glm::vec3(0.1f, 0.1f, 0.2f),
        .intensity = 1.0f
    };

    camera = std::make_shared<Camera>(
        glm::vec3(0.0f, 5.0f, 15.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        45.0f,
        1280.0f / 720.0f,
        0.1f,
        200.0f
    );

    PoliceCarFactory factory;
    factory.Initialize(phong);

    mainCar = factory.SpawnPlayerCar(phong);

    renderables.push_back(mainCar);
    updateables.push_back(mainCar);

    city = std::make_shared<CityModel>();

    Transform t = {
        .position = glm::vec3(5.0f, 0.0, 10.0f)
    };

    chaseCar = std::make_shared<CivilianCarActive>(
        phong,
        city->GenerateCity(phong, CITY_SIZE),
        t
    );

    chaseCar->SetPlayerPosition(mainCar->GetPosition());

    renderables.push_back(chaseCar);
    renderables.push_back(city);

    camera->SetTraget(std::weak_ptr<PoliceCarActive>(mainCar));
    updateables.push_back(camera);
    updateables.push_back(chaseCar);

    // This technically violates unique_ptr
    for (auto& light : mainCar->GetAllLights())
    {
        if (light->pointLight)
        {
            pointLights.push_back(light->pointLight.get());
        }
        if (light->spotLight)
        {
            spotLights.push_back(light->spotLight.get());
        }
    }

    for (auto& light : chaseCar->GetAllLights())
    {
        if (light->spotLight)
        {
            spotLights.push_back(light->spotLight.get());
        }
    }

    return true;
}

void Application::Run()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    double lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        // Get delta time.
        double currentFrame = glfwGetTime();
        double deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Poll events and handle input.
        glfwPollEvents();
        HandleUserInput(deltaTime);

        chaseCar->SetPlayerPosition(mainCar->GetPosition());

        // Update objects
        for (auto& obj : updateables)
        {
            obj->Update(deltaTime);
        }


        /* --- START SHADOW CUBES --- */

        auto& cubeShader = shaders[static_cast<size_t>(ShaderType::ShadowCube)];
        cubeShader->UseShader();

        constexpr float lightBarFarPlane = 25.0f;
        constexpr float blinkerFarPlane = 2.0f;
        constexpr float lightBarNearPlane = 0.01f;
        constexpr float blinkerNearPlane = 0.2f;

        for (size_t i = 0; i < shadowCubes.size(); ++i)
        {
            auto& shadowCube = shadowCubes[i];
            glm::vec3 lightPos;
            lightPos = mainCar->GetWorldPosOfLight(shadowCube.partIndex);
            float farPlane = 0.0f;
            float nearPlane = 0.0f;
            if (i == static_cast<size_t>(ShadowCubeIndex::Lightbar_L) || i == static_cast<size_t>(ShadowCubeIndex::Lightbar_R))
            {
                farPlane = lightBarFarPlane;
                nearPlane = lightBarNearPlane;
            }
            else
            {
                farPlane = blinkerFarPlane;
                nearPlane = blinkerNearPlane;
            }

            std::vector<glm::mat4> shadowTransforms = shadowCube.shadowCube->GetShadowMatrices(lightPos, nearPlane, farPlane);
            glViewport(0, 0, shadowCube.shadowCube->GetWidth(), shadowCube.shadowCube->GetHeight());
            // Bind and Upload Uniforms
            shadowCube.shadowCube->BindForWriting();
            glUniform3fv(cubeShader->GetUniformLocation("lightPos"), 1, &lightPos[0]);
            glUniform1f(cubeShader->GetUniformLocation("far_plane"), farPlane);

            for (unsigned int j = 0; j < 6; ++j) 
            {
                std::string name = "shadowMatrices[" + std::to_string(j) + "]";
                glUniformMatrix4fv(cubeShader->GetUniformLocation(name), 1, GL_FALSE, &shadowTransforms[j][0][0]);
            }

            // Draw scene onto cube map
            for (auto& r : renderables) 
            {
                r->Draw(cubeShader);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Now unbind everything

        /* --- STOP SHADOW CUBES --- */

        /* --- START SHADOW MAPS --- */

        std::array<glm::mat4, static_cast<size_t>(ShadowMapIndex::COUNT)> lightSpaceMatrices = {};

        auto& shadowShader = shaders[static_cast<size_t>(ShaderType::ShadowMap)];
        shadowShader->UseShader();
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);// Nudges the depth values during writing
        for (size_t i = 0; i < shadowMaps.size(); ++i)
        {
            auto& shadowMap = shadowMaps[i];
            glm::mat4 lsMatrix;
            glm::vec3 lightDir;
            glm::vec3 lightPos;
            float spotLightAngle = 0.0f;

            if (shadowMap.partIndex != PoliceCarPartIndex::None)
            {
                lightPos = mainCar->GetWorldPosOfLight(shadowMap.partIndex);
                spotLightAngle = mainCar->GetSpotlightOuterAngle(shadowMap.partIndex);
                lightDir = mainCar->GetForward() * static_cast<float>(shadowMap.forward);
            }
            else
            {
                lightPos = chaseCar->GetWorldPosOfLight(shadowMap.CV_partIndex);
                spotLightAngle = chaseCar->GetSpotlightOuterAngle(shadowMap.CV_partIndex);
                lightDir = chaseCar->GetForward() * static_cast<float>(shadowMap.forward);
            }

            lsMatrix = shadowMap.shadowMap->GetLightSpaceMatrix(lightDir, lightPos, spotLightAngle);
            lightSpaceMatrices[i] = lsMatrix;

            glViewport(0, 0, shadowMap.shadowMap->GetWidth(), shadowMap.shadowMap->GetHeight());
            shadowMap.shadowMap->BindForWriting();
            glUniformMatrix4fv(shadowShader->GetUniformLocation("lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrices[i][0][0]);
            for (auto& r : renderables) 
            {
                r->Draw(shadowShader);
            }
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Back to default buffer

        /* --- STOP SHADOW MAPS --- */

        /* --- START NORMAL DRAW --- */

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Clear screen once per frame
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = camera->GetProjectionMatrix();
        glm::vec3 camPos = camera->GetPosition();

        glm::mat4 projViewMatrix = projection * view;

        auto& s = shaders[static_cast<size_t>(ShaderType::Phong)];
        s->UseShader();

        glUniform1i(s->GetUniformLocation("fullBright"), isFullbright ? 1 : 0);

        glUniformMatrix4fv(s->GetUniformLocation("projViewMatrix"), 1, GL_FALSE, &projViewMatrix[0][0]);
        glUniform3fv(s->GetUniformLocation("viewPos"), 1, &camPos[0]);

        glUniformMatrix4fv(s->GetUniformLocation("headLMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::PL_Headlight_L)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("headRMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::PL_Headlight_R)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("rearLMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::PL_Rearlight_L)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("rearRMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::PL_Rearlight_R)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("reverseLMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::PL_Reverselight_L)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("reverseRMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::PL_Reverselight_R)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("highLMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::PL_Highbeam_L)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("highRMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::PL_Highbeam_R)][0][0]);

        glUniformMatrix4fv(s->GetUniformLocation("CV_headLMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::CV_Headlight_L)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("CV_headRMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::CV_Headlight_R)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("CV_rearLMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::CV_Rearlight_L)][0][0]);
        glUniformMatrix4fv(s->GetUniformLocation("CV_rearRMatrix"), 1, GL_FALSE, &lightSpaceMatrices[static_cast<size_t>(ShadowMapIndex::CV_Rearlight_R)][0][0]);

        for (size_t i = 0; i < shadowMaps.size(); ++i)
        {
            shadowMaps[i].shadowMap->BindForReading(static_cast<unsigned int>(i));
        }

        glUniform1i(s->GetUniformLocation("headlightLShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::PL_Headlight_L));
        glUniform1i(s->GetUniformLocation("headlightRShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::PL_Headlight_R));
        glUniform1i(s->GetUniformLocation("rearlightLShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::PL_Rearlight_L));
        glUniform1i(s->GetUniformLocation("rearlightRShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::PL_Rearlight_R));
        glUniform1i(s->GetUniformLocation("reverseLightLShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::PL_Reverselight_L));
        glUniform1i(s->GetUniformLocation("reverseLightRShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::PL_Reverselight_R));
        glUniform1i(s->GetUniformLocation("highBeamLShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::PL_Highbeam_L));
        glUniform1i(s->GetUniformLocation("highBeamRShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::PL_Highbeam_R));

        glUniform1i(s->GetUniformLocation("CV_headlightLShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::CV_Headlight_L));
        glUniform1i(s->GetUniformLocation("CV_headlightRShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::CV_Headlight_R));
        glUniform1i(s->GetUniformLocation("CV_rearlightLShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::CV_Rearlight_L));
        glUniform1i(s->GetUniformLocation("CV_rearlightRShadowMap"), SHADOW_MAP_UNIT_BASE + static_cast<unsigned int>(ShadowMapIndex::CV_Rearlight_R));

        for (size_t i = 0; i < shadowCubes.size(); ++i)
        {
            shadowCubes[i].shadowCube->BindForReading(static_cast<unsigned int>(i));
        }

        glUniform1i(s->GetUniformLocation("lightBarLShadowCube"), SHADOW_CUBE_UNIT_BASE + static_cast<unsigned int>(ShadowCubeIndex::Lightbar_L));
        glUniform1i(s->GetUniformLocation("lightBarRShadowCube"), SHADOW_CUBE_UNIT_BASE + static_cast<unsigned int>(ShadowCubeIndex::Lightbar_R));
        glUniform1i(s->GetUniformLocation("blinkerFLShadowCube"), SHADOW_CUBE_UNIT_BASE + static_cast<unsigned int>(ShadowCubeIndex::Blinker_FL));
        glUniform1i(s->GetUniformLocation("blinkerFRShadowCube"), SHADOW_CUBE_UNIT_BASE + static_cast<unsigned int>(ShadowCubeIndex::Blinker_FR));
        glUniform1i(s->GetUniformLocation("blinkerBLShadowCube"), SHADOW_CUBE_UNIT_BASE + static_cast<unsigned int>(ShadowCubeIndex::Blinker_BL));
        glUniform1i(s->GetUniformLocation("blinkerBRShadowCube"), SHADOW_CUBE_UNIT_BASE + static_cast<unsigned int>(ShadowCubeIndex::Blinker_BR));
        glUniform1f(s->GetUniformLocation("lightBarFarPlane"), lightBarFarPlane);
        glUniform1f(s->GetUniformLocation("blinkerFarPlane"), blinkerFarPlane);

        glUniform1i(s->GetUniformLocation("spotLightCount"), (int)spotLights.size());

        for (size_t i = 0; i < spotLights.size(); ++i)
        {
            spotLights[i]->Apply(*s, (int)i);
        }

        glUniform1i(s->GetUniformLocation("pointLightCount"), (int)pointLights.size());

        for (size_t i = 0; i < pointLights.size(); ++i)
        {
            pointLights[i]->Apply(*s, (int)i);
        }

        moon.Apply(*s);

        // Draw all renderables
        for (auto& r : renderables)
        {
            r->Draw();
        }

        /* --- STOP NORMAL DRAW --- */

        /* --- START XRAY --- */

        auto& xrayS = shaders[static_cast<size_t>(ShaderType::Xray)];
        float distance = glm::distance(mainCar->GetPosition(), chaseCar->GetPosition());

        // Xray baby!
        if (distance > 50.0f)
        {
            xrayS->UseShader();

            // Set-up Xray
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);   // DON'T WRITE TO DEPTH BUFFER YOU WILL RUIN EVERYTHING!!!!! (not really but it does look weird if you do)
            glDepthFunc(GL_GREATER); // ONLY draw if something is in front

            // Uniforms
            glUniformMatrix4fv(xrayS->GetUniformLocation("projViewMatrix"), 1, GL_FALSE, &projViewMatrix[0][0]);

            // Orange glow that gets more opaque the further away the positions are
            float alpha = glm::clamp((distance - 15.0f) / 100.0f, 0.2f, 0.7f);
            glUniform3f(xrayS->GetUniformLocation("glowColor"), 1.0f, 0.4f, 0.0f);
            glUniform1f(xrayS->GetUniformLocation("opacity"), alpha);

            // Draw car (and only this car!)
            chaseCar->Draw(xrayS);

            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        /* --- STOP XRAY --- */

        glfwSwapBuffers(window);
    }
}

void Application::Shutdown()
{
    renderables.clear();
    updateables.clear();
    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Application::HandleUserInput(double deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Direction::Forward, static_cast<float>(deltaTime));
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Direction::Backward, static_cast<float>(deltaTime));
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Direction::Left, static_cast<float>(deltaTime));
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Direction::Right, static_cast<float>(deltaTime));
    }

    // Mouse Input
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    camera->ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        mainCar->ProcessKeyboard(Direction::Forward, static_cast<float>(deltaTime));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        mainCar->ProcessKeyboard(Direction::Backward, static_cast<float>(deltaTime));
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        mainCar->ProcessKeyboard(Direction::Left, static_cast<float>(deltaTime));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        mainCar->ProcessKeyboard(Direction::Right, static_cast<float>(deltaTime));
    }

    if (keyHighBeam.JustPressed(window, GLFW_KEY_F)) 
    {
        mainCar->ToggleHighBeams();
    }

    if (keyHazard.JustPressed(window, GLFW_KEY_C)) 
    {
        mainCar->SetBlinkerState(BlinkerSide::HazardLights);
    }

    // Buttons are switched around for the user to be looking from the back, but the blinkers are treated as looking from the front.
    if (keyLeftBlinker.JustPressed(window, GLFW_KEY_E)) 
    {
        mainCar->SetBlinkerState(BlinkerSide::LeftSide);
    }

    if (keyRightBlinker.JustPressed(window, GLFW_KEY_Q)) 
    {
        mainCar->SetBlinkerState(BlinkerSide::RightSide);
    }

    if (keyLightbar.JustPressed(window, GLFW_KEY_G))
    {
        mainCar->ToggleLightBar();
    }

    if (keyShortBeam.JustPressed(window, GLFW_KEY_Z))
    {
        mainCar->ToggleShortBeams();
    }

    if (keyRearLights.JustPressed(window, GLFW_KEY_X))
    {
        mainCar->ToggleRearLights();
    }

    if (keyFullbright.JustPressed(window, GLFW_KEY_O))
    {
        isFullbright = !isFullbright;
    }

    if (keyCameraFree.JustPressed(window, GLFW_KEY_F1))
    {
        camera->SetMode(CameraMode::FreeRoam);
    }

    if (keyCameraFollow.JustPressed(window, GLFW_KEY_F2))
    {
        camera->SetMode(CameraMode::Follow);
    }

    if (keyCameraFirst.JustPressed(window, GLFW_KEY_F3))
    {
        camera->SetMode(CameraMode::FirstPerson);
    }
}
