#pragma once

#ifndef IRENDERABLE_H_
#define IRENDERABLE_H_

#include <memory>
#include "Shader.h"

/**
 * @brief Interface for objects that can be rendered.
 */
class IRenderable 
{
public:

    /**
    * @brief Default destructor.
    */
    virtual ~IRenderable() = default;

    /**
     * @brief Draw the object using the current shader.
     * 
     * @param customShader Custom shader to use. If null, uses subclass' shader.
     */
    virtual void Draw(std::shared_ptr<Shader> customShader = nullptr) const = 0;
};

#endif // IRENDERABLE_H_