//
// Created by Zara Hussain on 2023-04-20.
//

#include "PixelScene.h"

#include <vector>



PixelScene::PixelScene(VkDevice *device) : m_device(device)
{
    printf("PixelScene constructed\n");
}

void PixelScene::cleanup()
{

    free(modelTransferSpace);

    vkDestroyDescriptorPool(*m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout, nullptr);
    for(int i = 0; i < uniformBuffers.size(); i++)
    {
        vkDestroyBuffer(*m_device, dynamicUniformBuffers[i], nullptr);
        vkFreeMemory(*m_device, dynamicUniformBufferMemories[i], nullptr);
        vkDestroyBuffer(*m_device, uniformBuffers[i], nullptr);
        vkFreeMemory(*m_device, uniformBufferMemories[i], nullptr);
    }

    for(auto& object : allObjects)
    {
        object.cleanup();
    }
}

VkDescriptorSetLayout* PixelScene::getDescriptorSetLayout() {
    return &m_descriptorSetLayout;
}

VkDeviceSize PixelScene::getUniformBufferSize() {
    return sizeof(UboVP);
}

VkBuffer* PixelScene::getUniformBuffers(int index) {
    return &(uniformBuffers[index]);
}

VkDeviceMemory* PixelScene::getUniformBufferMemories(int index) {
    return &(uniformBufferMemories[index]);
}

void PixelScene::resizeBuffers(size_t newSize) {
    uniformBuffers.resize(newSize);
    uniformBufferMemories.resize(newSize);
    dynamicUniformBuffers.resize(newSize);
    dynamicUniformBufferMemories.resize(newSize);
    buffersUpdated.resize(newSize, false);
}

void PixelScene::addObject(PixelObject pixObject) {

    allObjects.push_back(pixObject);
}

int PixelScene::getNumObjects() {
    return allObjects.size();
}

PixelObject* PixelScene::getObjectAt(int index) {
    return &allObjects[index];
}

VkDescriptorPool *PixelScene::getDescriptorPool() {
    return &m_descriptorPool;
}

VkDescriptorSet* PixelScene::getDescriptorSetAt(int index) {
    return &m_descriptorSets[index];
}

void PixelScene::resizeDesciptorSets(size_t newSize) {
    m_descriptorSets.resize(newSize);
}

std::vector<VkDescriptorSet>* PixelScene::getDescriptorSets() {
    return &m_descriptorSets;
}

void PixelScene::updateUniformBuffer(uint32_t bufferIndex)
{
    bool bufferNeedUpdate = false;
    //check if any buffer is expired and need updating
    for(bool hasBufferUpdated : buffersUpdated)
    {
        if(!hasBufferUpdated)
        {
            bufferNeedUpdate = true;
            break;
        }
    }

    if(bufferNeedUpdate)
    {
        sceneVP.P[1][1] *= -1; //invert the y scale to flip the image. Vulkan is flipped by default

        void* data;
        vkMapMemory(*m_device, uniformBufferMemories[bufferIndex], 0, getUniformBufferSize(),0,&data);
        memcpy(data, &sceneVP, getUniformBufferSize());
        vkUnmapMemory(*m_device, uniformBufferMemories[bufferIndex]);

        buffersUpdated[bufferIndex] = true;
    }

}

void PixelScene::createDescriptorSetLayout() {

    //how data is bound to the shader in binding 0
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
    descriptorSetLayoutBinding.binding = 0; //binding point in shader
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBinding.descriptorCount = 1; //only binding one uniform buffer
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

    //how data is bound to the shader in binding 0
    VkDescriptorSetLayoutBinding modelLayoutBinding{};
    modelLayoutBinding.binding = 1; //binding point in shader
    modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    modelLayoutBinding.descriptorCount = 1; //only binding one uniform buffer
    modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    modelLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding,2> descriptorSetLayoutBindings = {descriptorSetLayoutBinding, modelLayoutBinding};

    //Create descriptor set layout given binding
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());

    VkResult result = vkCreateDescriptorSetLayout(*m_device, &layoutCreateInfo, nullptr, &m_descriptorSetLayout);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

PixelScene::UboVP PixelScene::getSceneVP() {
    return sceneVP;
}

void PixelScene::setSceneVP(PixelScene::UboVP vpData)
{
    sceneVP = vpData;
}

void PixelScene::setSceneV(glm::mat4 V) {
    sceneVP.V = glm::mat4(V);
}

void PixelScene::setSceneP(glm::mat4 P) {
    sceneVP.P = glm::mat4(P);
}

bool PixelScene::areMatricesEqual(glm::mat4 x, glm::mat4 y) {

    for(int i = 0; i < 4 ; i++)
    {
        for(int j = 0; j < 4 ; j++)
        {
            if(abs(x[i][j] - y[i][j]) >= 0.000001f)
            {
                return false;
            }
        }
    }

    return true;
}

void PixelScene::allocateDynamicBufferTransferSpace()
{
    //calculate allignment
    objectUBOAllignment = (sizeof(PixelObject::DynamicUBObj) + minUBOOffset - 1) & ~(minUBOOffset - 1); //right portion is our mask

    //create memory for all the objects dynamic buffers;
    modelTransferSpace = (PixelObject::DynamicUBObj*) aligned_alloc(minUBOOffset, objectUBOAllignment * MAX_OBJECTS);
}

void PixelScene::getMinUBOOffset(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    minUBOOffset = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
}

VkBuffer *PixelScene::getDynamicUniformBuffers(int index) {
    return &(dynamicUniformBuffers[index]);
}

VkDeviceMemory *PixelScene::getDynamicUniformBufferMemories(int index) {
    return &(dynamicUniformBufferMemories[index]);
}

VkDeviceSize PixelScene::getDynamicUniformBufferSize() {
    return objectUBOAllignment * MAX_OBJECTS;
}

VkDeviceSize PixelScene::getMinAlignment() {
    return objectUBOAllignment;
}

void PixelScene::updateDynamicUniformBuffer(uint32_t bufferIndex) {
    for(size_t i = 0; i<allObjects.size(); i++)
    {
        auto* currentPushM = (PixelObject::DynamicUBObj*)((uint64_t)modelTransferSpace + (i * objectUBOAllignment));
        *currentPushM = *(allObjects[i].getDynamicUBObj());
    }

    //map the whole chunk of memory data
    void* data;
    vkMapMemory(*m_device, dynamicUniformBufferMemories[bufferIndex], 0, objectUBOAllignment * allObjects.size() , 0 , &data);
    memcpy(data, modelTransferSpace, objectUBOAllignment * allObjects.size());
    vkUnmapMemory(*m_device, dynamicUniformBufferMemories[bufferIndex]);
}

void PixelScene::initialize(VkPhysicalDevice physicalDevice) {
    getMinUBOOffset(physicalDevice);
    allocateDynamicBufferTransferSpace();
    createDescriptorSetLayout();
}
