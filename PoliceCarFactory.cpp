#include "PoliceCarFactory.h"

void PoliceCarFactory::Initialize(const std::weak_ptr<Shader>& shader)
{
    if (sharedModelData)
    {
        return;
    }

    sharedModelData = std::make_shared<PoliceCarModelData>();

    PoliceCarModel builder(shader, sharedModelData);
}

std::shared_ptr<PoliceCarModel> PoliceCarFactory::SpawnModel(const Transform& t)
{
    return std::make_shared<PoliceCarModel>(sharedModelData, t);
}

std::shared_ptr<PoliceCarActive> PoliceCarFactory::SpawnPlayerCar(const std::weak_ptr<Shader>& shader, const Transform& t)
{
    auto player = std::make_shared<PoliceCarActive>(shader, t);
    return player;
}
