#pragma once

#ifndef CITYPART_H_
#define CITYPART_H_

#include "BuildingMeshType.h"
#include "BuildingPartType.h"

struct CityPart
{
	std::unique_ptr<Mesh> mesh;	/// Mesh of the part.
	BuildingMeshType meshType;	/// Type of mesh.
	BuildingPartType partType;	/// Type of the part.
};

#endif // CITYPART_H_