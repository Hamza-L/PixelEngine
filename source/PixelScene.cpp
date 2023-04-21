//
// Created by Zara Hussain on 2023-04-20.
//

#include "PixelScene.h"

#include <vector>



PixelScene::PixelScene(VkDevice *device) : m_device(device){
    printf("PixelScene constructed\n");

}

void PixelScene::cleanup() {
    vkDestroyDescriptorPool(*m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, nullptr);
    for(int i = 0; i < uniformBuffers.size(); i++)
    {
        vkDestroyBuffer(*m_device, uniformBuffers[i], nullptr);
        vkFreeMemory(*m_device, uniformBufferMemories[i], nullptr);
    }

    for(auto& object : allObjects)
    {
        object->cleanup();
    }
}

VkDescriptorSetLayout* PixelScene::getDescriptorSetLayout() {
    return &m_descriptorSetLayout;
}

VkDeviceSize PixelScene::getUniformBufferSize() {
    return sizeof(MVP);
}

VkBuffer* PixelScene::getUniformBuffers(int index) {
    return &(uniformBuffers[index]);
}

VkDeviceMemory* PixelScene::getUniformBufferMemories(int index) {
    return &(uniformBufferMemories[index]);
}

void PixelScene::resizeBuffers(int newSize) {
    uniformBuffers.resize(newSize);
    uniformBufferMemories.resize(newSize);
}

void PixelScene::addObject(const std::shared_ptr<PixelObject>& pixObject) {

    allObjects.push_back(pixObject);
}

int PixelScene::getNumObjects() {
    return allObjects.size();
}

PixelObject* PixelScene::getObjectAt(int index) {
    return allObjects[index].get();
}

PixelScene::PixelScene() {
    printf("PixelScene default constructed\n");
}

VkDescriptorPool *PixelScene::getDescriptorPool() {
    return &m_descriptorPool;
}

VkDescriptorSet* PixelScene::getDescriptorSetAt(int index) {
    return &m_descriptorSets[index];
}

void PixelScene::resizeDesciptorSets(int newSize) {
    m_descriptorSets.resize(newSize);
}

std::vector<VkDescriptorSet>* PixelScene::getDescriptorSets() {
    return &m_descriptorSets;
}
