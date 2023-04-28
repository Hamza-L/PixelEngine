//
// Created by Zara Hussain on 2023-04-14.
//

#ifndef PIXELENGINE_PIXELOBJECT_H
#define PIXELENGINE_PIXELOBJECT_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <string>
#include <array>
#include <vector>

class PixelObject {
public:

    // this should noe change every frame, but can change per individual object/mesh.
    //Dynamic Uniform Buffer Object
    struct DynamicUBObj{
        glm::mat4 M{};
        glm::mat4 MinvT{};
    };

    // this can change every frame, and can change per individual object/mesh.
    //Dynamic Uniform Buffer Object
    struct PObj{
        glm::mat4 M{};
        glm::mat4 MinvT{};
    };

    //the vertex must only contain member variables of type vec4 (each 16 bytes)
    struct Vertex
    {
        glm::vec4 position{};
        glm::vec4 normal{};
        glm::vec4 color{};
    };

    PixelObject(VkDevice* device, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
    PixelObject(VkDevice* device, std::string filename);

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
    PObj* getPushObj();
    DynamicUBObj* getDynamicUBObj();
    static constexpr VkPushConstantRange pushConstantRange {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PObj)};

    //setters
    void setDynamicUBObj(DynamicUBObj pushObjData);
    void setPushObj(PObj pushObjData);

    //cleanup
    void cleanup();


    //helper functions
    //returns the number of members of the Vertex Struct
    static constexpr int getNumofAttributes(){return sizeof(Vertex)/sizeof(Vertex::position);}
    void importFile(const std::string& filename);
    void setGenericColor(glm::vec4 color);
    void addTransform(glm::mat4 matTransform);


private:
    //member variables
    std::vector<Vertex> m_vertices{};
    std::vector<uint32_t> m_indices{};
    std::string name{};

    //transforms
    DynamicUBObj dynamicUBO = {glm::mat4(1.0f)};
    PObj pushObj = {glm::mat4(1.0f)};

    //vulkan components
    VkDevice* m_device = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;


};


#endif //PIXELENGINE_PIXELOBJECT_H
