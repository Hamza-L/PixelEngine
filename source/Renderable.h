#ifndef RENDERABLE_H_
#define RENDERABLE_H_

#include "PixelRenderer.h"

// rendereable interface
class Renderable{
public:
    Renderable(){};
    ~Renderable(){};
    virtual void Build() = 0;
    virtual void Render() = 0;
};

#endif // RENDERABLE_H_
