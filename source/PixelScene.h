//
// Created by Zara Hussain on 2023-04-20.
//

#ifndef PIXELENGINE_PIXELSCENE_H
#define PIXELENGINE_PIXELSCENE_H

#include "PixelObject.h"
#include "glm/glm.hpp"

static const glm::mat4 MAT4_IDENTITY = {1,0,0,0,
                                        0,1,0,0,
                                        0,0,1,0,
                                        0,0,0,1};

class PixelScene {
public:
    PixelScene(VkDevice* device);
    PixelScene();
    PixelScene(const PixelScene&) = delete;

    //setter functions
    void addObject(const std::shared_ptr<PixelObject>& pixObject);

    //getter functions
    VkDescriptorSetLayout* getDescriptorSetLayout();
    VkDescriptorPool* getDescriptorPool();
    VkDescriptorSet* getDescriptorSetAt(int index);
    std::vector<VkDescriptorSet>* getDescriptorSets();
    static VkDeviceSize getUniformBufferSize();
    VkBuffer* getUniformBuffers(int index);
    VkDeviceMemory* getUniformBufferMemories(int index);
    int getNumObjects();
    PixelObject* getObjectAt(int index);

    //helper functions
    void resizeBuffers(int newSize);
    void resizeDesciptorSets(int newSize);

    //cleanup
    void cleanup();
private:
    struct MVP{
        glm::mat4 M{};
        glm::mat4 V{};
        glm::mat4 P{};
    };

    //objects
    std::vector<std::shared_ptr<PixelObject>> allObjects;

    //------UNIFORM BUFFER
    MVP sceneMVP; //model view projection matrix
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemories;

    //------TEXTURES


    //vulkan component
    VkDevice* m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;
};


#endif //PIXELENGINE_PIXELSCENE_H
