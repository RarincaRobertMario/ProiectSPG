#pragma once

#ifndef CIVILIANCARPART_H_
#define CIVILIANCARPART_H_

#include <memory>

#include "CivilianCarMesh.h"
#include "CivilianCarLight.h"

/**
* @brief Structure for a civilian car part.
*/
struct CivilianCarPart
{
	std::unique_ptr<CivilianCarMesh> meshData;		/// Mesh data.
	std::unique_ptr<CivilianCarLight> lightData;	/// Light data.
};

#endif // CIVILIANCARPART_H_