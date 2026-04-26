#pragma once

#ifndef BUILDINGMESHTYPE_H_
#define BUILDINGMESHTYPE_H_

/**
* @brief Enumeration of building mesh types.
*/
enum class BuildingMeshType 
{
    Solid,  /// Solid.
    Alpha,  /// Alpha modulated (windows).
    Light,  /// Lights.
    COUNT   /// Number of elements.
};

#endif // BUILDINGMESHTYPE_H_