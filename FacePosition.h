#pragma once

#ifndef FACEPOSITION_H_
#define FACEPOSITION_H_

#include <type_traits>
#include <cstdint>

#define FACE_POSITION   \
    X(Left)             \
    X(Right)            \
    X(Center)           \
    X(Front)            \
    X(Behind)           \
    X(Top)              \
    X(Bottom)           \

/**
* @brief Indexing enum.
*/
enum class FacePositionIndex : uint8_t
{
#define X(name) name,
    FACE_POSITION
#undef X
};

/**
* @brief Enumerations of where an element is on a square face.
*
* These are interpreted as you looking at the square, while looking in the positive Z axis.
* 
*                    [Front view]
* |--------------|-------------------|----------------|
* |Top|Left      |  Top(|Center)     | Top|Right      |
* |--------------|-------------------|----------------|
* |(Center|)Left |   Center          | (Center|)Right |
* |--------------|-------------------|----------------|
* |Bottom|Left   |   Bottom(|Center) | Bottom|Right   |
* |--------------|-------------------|----------------|
* 
*                    [Side view]
* 
*       Front|Top       |       Behind|Top
*                       |
*       Front           |       Behind
*                       |
*       Front|Bottom    |       Behind|Bottom
* 
*/
enum class FacePosition : uint8_t
{
#define X(name) name = 1 << static_cast<uint8_t>(FacePositionIndex::name),
    FACE_POSITION
#undef X
    /*
        Left,   /// Left side of the square face.
        Right,  /// Right side of the square face.
        Center  /// Center of the square face.
        Front,  /// Front of the square face, towards +inf.
        Behind, /// Back of the square face, towards -inf.
        Top,    /// Top of the face.
        Bottom  /// Bottom of the face.
    */
};

using FacePositionUnderlying = std::underlying_type_t<FacePosition>;

/**
* @brief OR operator.
*/
constexpr inline FacePosition operator|(FacePosition a, FacePosition b) {
    return static_cast<FacePosition>(
        static_cast<FacePositionUnderlying>(a) | static_cast<FacePositionUnderlying>(b)
        );
}

/**
* @brief AND operator.
*/
constexpr inline FacePosition operator&(FacePosition a, FacePosition b) {
    return static_cast<FacePosition>(
        static_cast<FacePositionUnderlying>(a) & static_cast<FacePositionUnderlying>(b)
        );
}

/**
* @brief Logical NOT operator.
*/
constexpr inline FacePosition operator~(FacePosition a) {
    return static_cast<FacePosition>(
        ~static_cast<FacePositionUnderlying>(a)
        );
}

/**
* @brief Assign OR operator.
*/
constexpr inline FacePosition& operator|=(FacePosition& a, FacePosition b) {
    a = a | b;
    return a;
}

/**
* @brief Assign AND operator.
*/
constexpr inline FacePosition& operator&=(FacePosition& a, FacePosition b) {
    a = a & b;
    return a;
}

/**
* @brief Checks if a FacePosition contains a particular flag or more.
*
* @param flags Flags to test.
* @param test Flags to check for.
*
* @return 'true' if 'flags' contains atleast one tested flag, 'false' if it contains to flags tested for.
*/
constexpr inline bool HasFlag(FacePosition flags, FacePosition test) { return (flags & test) != static_cast<FacePosition>(0); }


#endif // FACEPOSITION_H_