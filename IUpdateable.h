#pragma once

#ifndef IUPDATEABLE_H_
#define IUPDATEABLE_H_


/**
 * @brief Interface for objects that need per-frame updates.
 */
class IUpdateable 
{
public:

    /**
    * @brief Default destructor.
    */
    virtual ~IUpdateable() = default;

    /**
     * @brief Update object logic with delta time.
     * 
     * @param deltaTime Time elapsed since last frame (seconds)
     */
    virtual void Update(double deltaTime) = 0;
};

#endif // IUPDATEABLE_H_