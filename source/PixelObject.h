//
// Created by Zara Hussain on 2023-04-14.
//

#ifndef PIXELENGINE_PIXELOBJECT_H
#define PIXELENGINE_PIXELOBJECT_H

#include "glm/glm.hpp"
#include <string>
#include <array>

class PixelObject {
public:

    //the vertex must only contain member variables of type vec4 (each 16 bytes)
    struct Vertex
    {
        glm::vec4 position{};
        //glm::vec4 color{};
    };

    //returns the number of members of the Vertex Struct
    static constexpr int getNumofAttributes(){return sizeof(Vertex)/sizeof(Vertex::position);}

};


#endif //PIXELENGINE_PIXELOBJECT_H
