#pragma once

#ifndef DIRECTION_H_
#define DIRECTION_H_

/**
* @brief Enumeration of movement directions.
*/
enum Direction : uint8_t
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down,
    COUNT   /// Number of elements.
};

#endif // DIRECTION_H_