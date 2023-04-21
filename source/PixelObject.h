//
// Created by Zara Hussain on 2023-04-14.
//

#ifndef PIXELENGINE_PIXELOBJECT_H
#define PIXELENGINE_PIXELOBJECT_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include <string>
#include <array>
#include <vector>

class PixelObject {
public:
    //the vertex must only contain member variables of type vec4 (each 16 bytes)
    struct Vertex
    {
        glm::vec4 position{};
        glm::vec4 color{};
    };

    PixelObject(VkDevice* device, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
    PixelObject(const PixelObject&) = delete;

    //returns the number of members of the Vertex Struct
    static constexpr int getNumofAttributes(){return sizeof(Vertex)/sizeof(Vertex::position);}

    //getters
    int getVertexCount();
    std::vector<Vertex>* getVertices();
    VkDeviceSize getVertexBufferSize();
    VkBuffer* getVertexBuffer();
    VkDeviceMemory* getVertexBufferMemory();
    int getIndexCount();
    std::vector<uint32_t>* getIndices();
    VkDeviceSize getIndexBufferSize();
    VkBuffer* getIndexBuffer();
    VkDeviceMemory* getIndexBufferMemory();

    //cleanup
    void cleanup();

private:
    //member variables
    std::vector<Vertex> m_vertices{};
    std::vector<uint32_t> m_indices{};
    std::string name{};
    VkDevice* m_device = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
};


#endif //PIXELENGINE_PIXELOBJECT_H
