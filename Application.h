#pragma once

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <GLFW/glfw3.h>
#include <vector>

#include "IUpdateable.h"
#include "IRenderable.h"
#include "Camera.h"
#include "Spotlight.h"
#include "PointLight.h"
#include "PoliceCar.h"
#include "DirLight.h"
#include "ShadowMap.h"
#include "ShadowCube.h"
#include "ShadowCubeIndex.h"
#include "KeyState.h"
#include "ShaderType.h"
#include "PoliceCarActive.h"
#include "CityModel.h"
#include "CivilianCarActive.h"

struct GLFWwindow;
class Shader;
class Triangle;

/**
* @brief Main controller class for the application.
*/
class Application 
{
private:
    GLFWwindow* window = nullptr;     /// Pointer to the window.
    std::vector<std::shared_ptr<IRenderable>> renderables;  /// Vector of renderable elements.
    std::vector<std::shared_ptr<IUpdateable>> updateables;  /// Vector of updateable elements.
    std::vector<SpotLight*> spotLights;    /// Vector of spotlights references. DOES NOT OWN THEM!
    std::vector<PointLight*> pointLights;  /// Vector of point light references. DOES NOT OWN THEM!
    std::array<std::shared_ptr<Shader>, static_cast<size_t>(ShaderType::COUNT)> shaders; /// Array of shaders.

    /**
    * @brief Structure for a shadow map map entry.
    */
    struct ShadowMapEntry
    {
        std::unique_ptr<ShadowMap> shadowMap;                       /// Shadow map itself.
        int8_t forward = 1;                                         /// '1' if the light is in front of the car, -1 for 'behind' the car.
        PoliceCarPartIndex partIndex = PoliceCarPartIndex::None;    /// Police car part of the shadow map.
        CivilianCarPartIndex CV_partIndex = CivilianCarPartIndex::None; /// Civilian car part index.
    };

    std::array<ShadowMapEntry, static_cast<size_t>(ShadowMapIndex::COUNT)> shadowMaps;  /// Array of shadow maps.

    /**
    * @brief Structure for a shadow cube map entry.
    */
    struct ShadowCubeEntry
    {
        std::unique_ptr<ShadowCube> shadowCube;                     /// Shadow cube itself.
        PoliceCarPartIndex partIndex = PoliceCarPartIndex::None;    /// Police car part of the shadow cube.
    };

    std::array<ShadowCubeEntry, static_cast<size_t>(ShadowCubeIndex::COUNT)> shadowCubes;   /// Array of shadow cubes.

    std::shared_ptr<Camera> camera;  /// Camera.
    DirLight moon;  /// Moon light

    std::shared_ptr<PoliceCarActive> mainCar; /// Main police car.
    std::shared_ptr<CityModel> city;    /// City model.
    std::shared_ptr<CivilianCarActive> chaseCar;    /// Fleeing car.

    KeyState keyHighBeam;       /// Key state for the high beam.
    KeyState keyLeftBlinker;    /// Key state for the left blinkers.
    KeyState keyRightBlinker;   /// Key state for the right blinkers.
    KeyState keyHazard;         /// Key state for the hazard lights.
    KeyState keyLightbar;       /// Key state for the light bar.
    KeyState keyShortBeam;      /// Key state for the short beam.
    KeyState keyRearLights;     /// Key state for the rear lights.
    KeyState keyFullbright;     /// Key state for full bright mode.
    KeyState keyCameraFree;     /// Key state for setting the camera to free mode.
    KeyState keyCameraFollow;   /// Key state for setting the camera to follow mode.
    KeyState keyCameraFirst;    /// Key state for setting the camera to first person mode.

    bool isFullbright = false; /// Flag if the scene should be full bright or not.

    // No copying.
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
    * @brief Handles user input.
    */
    void HandleUserInput(double deltaTime);

public:

    /**
    * @brief Default constructor.
    */
    Application() = default;

    /**
    * @brief Default move constructor.
    */
    Application(Application&& other) noexcept = default;

    /**
     * @brief Destructor. Frees resources.
     */
    ~Application();

    /**
    * @brief Initialises the OpenGL systems (GLAD, GLFW, etc).
    */
    bool Init();

    /**
    * @brief Runs the main app.
    */
    void Run();

    /**
    * @brief Shuts the app down and cleans-up resources.
    */
    void Shutdown();

    /**
    * @brief Default move operator.
    */
    Application& operator=(Application&& other) noexcept = default;
};

#endif // APLICATION_H_