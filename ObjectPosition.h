#pragma once

#ifndef OBJECTPOSITION_H_
#define OBJECTPOSITION_H_

#include "MeshPosition.h"
#include "FacePosition.h"

/**
* @brief Structure for a position of an element relative to another one or a point.
* 
* Has no meaning on its own.
*/
struct ObjectPosition
{
	MeshPosition meshPosition = MeshPosition::Center;	/// Where relative to the mesh (cube) the object is.
	FacePosition facePosition = FacePosition::Center;	/// Where on the face of the mesh (square) the object is.
};

#endif // OBJECTPOSITION_H_