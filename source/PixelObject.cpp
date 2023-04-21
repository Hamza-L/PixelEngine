//
// Created by Zara Hussain on 2023-04-14.
//

#include "PixelObject.h"

#include <utility>


PixelObject::PixelObject(VkDevice* device, std::vector<Vertex> vertices, std::vector<uint32_t> indices): m_device(device), m_vertices(std::move(vertices)), m_indices(std::move(indices)) {
    //create the vertex buffer form the vertices
    printf("PixelObject constructed\n");
    //createVertexBuffer(vertices);
}

void PixelObject::cleanup() {
    vkFreeMemory(*m_device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(*m_device, vertexBuffer, nullptr);
    vkFreeMemory(*m_device, indexBufferMemory, nullptr);
    vkDestroyBuffer(*m_device, indexBuffer, nullptr);
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

