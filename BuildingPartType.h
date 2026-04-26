#pragma once

#ifndef BUILDINGPARTTYPE_H_
#define BUILDINGPARTTYPE_H_

/**
* @brief Enumeration of building part types
*/
enum class BuildingPartType 
{
    Foundation,     /// Foundation.
    Wall,           /// Wall.
    Window,         /// Window.
    Roof,           /// Roof.
    Sidewalk,       /// Sidewalk.
    Light,          /// Light (any).
    COUNT           /// Number of elements
};

#endif // BUILDINGPARTTYPE_H_