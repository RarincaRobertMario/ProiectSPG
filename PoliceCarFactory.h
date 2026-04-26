#pragma once

#ifndef POLICECARFACTORY_H_
#define POLICECARFACTORY_H_

#include "PoliceCarActive.h"

/**
* @brief A factory for police cars, holds the shared data for models.
*/
class PoliceCarFactory 
{
private:
    std::shared_ptr<PoliceCarModelData> sharedModelData;  /// Shared data.

public:

    /**
    * @brief Initialises the factory and creates the shared data.
    * 
    * @param shader Weak pointer t the shader used.
    */
    void Initialize(const std::weak_ptr<Shader>& shader)
    {
        if (sharedModelData)
        {
            return;
        }

        sharedModelData = std::make_shared<PoliceCarModelData>();

        PoliceCarModel builder(shader, sharedModelData);
    }

    /**
    * @brief Spawns a model.
    * 
    * @param t Transform of the mode.
    * 
    * @return Shared pointer to the model.
    */
    std::shared_ptr<PoliceCarModel> SpawnModel(const Transform& t = {})
    {
        return std::make_shared<PoliceCarModel>(sharedModelData, t);
    }

    /**
    * @brief Spawns the main controllable police car.
    * 
    * @param shader Shader to be used.
    * @param t Optional transform
    * 
    * @return Shared pointer to the 'active' police car.
    */
    std::shared_ptr<PoliceCarActive> SpawnPlayerCar(const std::weak_ptr<Shader>& shader, const Transform& t = {})
    {
        auto player = std::make_shared<PoliceCarActive>(shader, t);
        return player;
    }
};
#endif // POLICECARFACTORY_H_