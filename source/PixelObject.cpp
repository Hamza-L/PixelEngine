//
// Created by Zara Hussain on 2023-04-14.
//

#include "PixelObject.h"

#include <utility>
#include <fstream>


PixelObject::PixelObject(std::vector<Vertex> vertices, std::vector<uint32_t> indices): m_vertices(vertices), m_indices(indices) {
    //create the vertex buffer form the vertices
    LOG(Level::DEBUG, "PixelObject Constructed");
    //createVertexBuffer(vertices);
}

PixelObject::PixelObject(std::string filename){
    importFile(filename);
}

void PixelObject::cleanup(PixBackend* device) {

    for(auto texture : m_textures)
    {
        if(!texture.hasBeenCleaned())
        {
            texture.cleanUp(device);
        }
    }

    vkFreeMemory(device->logicalDevice, vertexBufferMemory, nullptr);
    vkDestroyBuffer(device->logicalDevice, vertexBuffer, nullptr);
    vkFreeMemory(device->logicalDevice, indexBufferMemory, nullptr);
    vkDestroyBuffer(device->logicalDevice, indexBuffer, nullptr);
}

std::vector<PixelObject::Vertex>* PixelObject::getVertices() {
    return &m_vertices;
}

int PixelObject::getVertexCount() {
    return m_vertices.size();
}

VkDeviceSize PixelObject::getVertexBufferSize() {
    return (sizeof(Vertex) * m_vertices.size());
}

VkBuffer* PixelObject::getVertexBuffer() {
    return &vertexBuffer;
}

VkDeviceMemory *PixelObject::getVertexBufferMemory() {
    return &vertexBufferMemory;
}

std::vector<uint32_t> *PixelObject::getIndices() {
    return &m_indices;
}

int PixelObject::getIndexCount() {
    return m_indices.size();
}

VkDeviceSize PixelObject::getIndexBufferSize() {
    return (sizeof(uint32_t) * m_indices.size());
}

VkBuffer *PixelObject::getIndexBuffer() {
    return &indexBuffer;
}

VkDeviceMemory *PixelObject::getIndexBufferMemory() {
    return &indexBufferMemory;
}

void PixelObject::setDynamicUBObj(DynamicUBObj pushObjData) {
    dynamicUBO = pushObjData;
}

PixelObject::PObj* PixelObject::getPushObj() {
    return &pushObj;
}

void PixelObject::importFile(const std::string& filename) {
    std::string fileLocation = "objects/" + filename;
}

void PixelObject::addTransform(glm::mat4 matTransform) {
    pushObj.M = matTransform * pushObj.M;
    pushObj.MinvT = glm::transpose(glm::inverse(pushObj.M));
}

void PixelObject::setTransform(glm::mat4 matTransform) {
    pushObj.M = matTransform;
    pushObj.MinvT = glm::transpose(glm::inverse(pushObj.M));
}

void PixelObject::setPushObj(PixelObject::PObj pushObjData) {
    pushObj = PObj(pushObjData);
}

PixelObject::DynamicUBObj* PixelObject::getDynamicUBObj() {
    return &dynamicUBO;
}

void PixelObject::addTexture(PixBackend* devices, std::string textureFile) {
    PixelImage textureImage = PixelImage(0, 0, false);
    textureImage.loadTexture(devices, textureFile);

    setTexID(0);

    m_textures.push_back(textureImage);

}

void PixelObject::addTexture(PixelImage* pixImage) {

    setTexID(0);

    m_textures.push_back(*pixImage);

}

