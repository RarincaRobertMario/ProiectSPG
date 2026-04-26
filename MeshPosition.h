#pragma once

#ifndef MESHPOSITION_H_
#define MESHPOSITION_H_

#include <type_traits>
#include <cstdint>

#define MESH_POSITIONS \
    X(Left)        \
    X(Right)       \
    X(Front)       \
    X(Back)        \
    X(Center)      \
    X(Above)       \
    X(Below)

/**
* @brief Indexes.
*/
enum class MeshPositionIndex : uint8_t
{
#define X(name) name,
    MESH_POSITIONS
#undef X
};

/**
* @brief Enumerations of where an element is relative to a mesh.
* 
* These are interpreted as the object being with the center in (0,0,0) and looking towards the positive Z axis.
*/
enum class MeshPosition : uint8_t
{
#define X(name) name = 1 << static_cast<uint8_t>(MeshPositionIndex::name),
    MESH_POSITIONS
#undef X
    /*
        Left,   /// Left side of the mesh.
        Right,  /// Right side of the mesh.
        Front,  /// Front of the mesh.
        Back,   /// Back of the mesh.
        Center, /// Center of the mesh.
        Above,  /// Above the mesh, towards +inf.
        Below   /// Below the mesh, towards -inf.
    */
};

using MeshPositionUnderlying = std::underlying_type_t<MeshPosition>;

/**
* @brief OR operator.
*/
constexpr inline MeshPosition operator|(MeshPosition a, MeshPosition b) {
    return static_cast<MeshPosition>(
        static_cast<MeshPositionUnderlying>(a) | static_cast<MeshPositionUnderlying>(b)
    );
}

/**
* @brief AND operator.
*/
constexpr inline MeshPosition operator&(MeshPosition a, MeshPosition b) {
    return static_cast<MeshPosition>(
        static_cast<MeshPositionUnderlying>(a) & static_cast<MeshPositionUnderlying>(b)
    );
}

/**
* @brief Logical NOT operator.
*/
constexpr inline MeshPosition operator~(MeshPosition a) {
    return static_cast<MeshPosition>(
        ~static_cast<MeshPositionUnderlying>(a)
    );
}

/**
* @brief Assign OR operator.
*/
constexpr inline MeshPosition& operator|=(MeshPosition& a, MeshPosition b) {
    a = a | b;
    return a;
}

/**
* @brief Assign AND operator.
*/
constexpr inline MeshPosition& operator&=(MeshPosition& a, MeshPosition b) {
    a = a & b;
    return a;
}

/**
* @brief Checks if a MeshPosition contains a particular flag or more.
* 
* @param flags Flags to test.
* @param test Flags to check for.
* 
* @return 'true' if 'flags' contains atleast one tested flag, 'false' if it contains to flags tested for.
*/
constexpr inline bool HasFlag(MeshPosition flags, MeshPosition test) { return (flags & test) != static_cast<MeshPosition>(0); }


#endif // MESHPOSITION_H_