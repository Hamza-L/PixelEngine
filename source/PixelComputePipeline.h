//
// Created by hlahm on 2023-05-31.
//

#ifndef PIXELENGINE_PIXELCOMPUTEPIPELINE_H
#define PIXELENGINE_PIXELCOMPUTEPIPELINE_H

#include "PixelImage.h"
#include "glm/glm.hpp"

class PixelComputePipeline {
public:
    PixelComputePipeline() = default;

    struct PObj{
        glm::vec3 cameraPos;
        float fov;
        glm::vec3 randomOffsets;
        float focus;
        glm::vec3 lightPos;
        float intensity;
        glm::vec4 lightColor;
        uint32_t currentSample;
        uint32_t mouseCoordX;
        uint32_t mouseCoordY;
        uint32_t outlineEnabled;
    };

    void addComputeShader(PixBackend* devices, const std::string& filename);
    void createDescriptorPool(PixBackend* devices);
    void createDescriptorSets(PixBackend* devices);
    void initImageBufferStorage(PixBackend* devices);
    void populatePipelineLayout();
    void createDescriptorSetLayout(PixBackend* devices);
    void createComputePipeline(PixBackend* devices);
    void createComputePipelineLayout(PixBackend* devices);
    void init(PixBackend* devices);
    void cleanUp(PixBackend* devices);
    static constexpr VkPushConstantRange pushComputeConstantRange {VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PObj)};

    //getters
    VkPipeline getPipeline();
    VkPipelineLayout getPipelineLayout();
    VkDescriptorSet getDescriptorSet();
    PixelImage* getInputTexture();
    PixelImage* getOutputTexture();
    PixelImage* getCustomTexture();
    PObj* getPushObj(){return &test;}

    //setters
    void setPushObj(PixelComputePipeline::PObj pObj){test = pObj;}

private:

    PixelImage raytracedInputTexture;
    PixelImage raytracedOutputTexture;
    PixelImage customTexture;

    PObj test = {{0.0f,1.0f,5.0f},35.0f,{0.0f,0.0f,0.0f},0.0f, {3.0f,4.0f,0.0f},0.0f,{1.0f,1.0f,1.0f,1.0f}, 0, 0, 0, 0};

    VkPipelineShaderStageCreateInfo computeCreateShaderInfo{};
    VkPipeline computePipeline = VK_NULL_HANDLE;
    VkPipelineLayout computePipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayoutCreateInfo computePipelineLayoutCreateInfo = {};
    VkShaderModule computeShaderModule = VK_NULL_HANDLE;
    VkDescriptorSetLayout computeDescriptorSetLayout{};
    VkDescriptorSet computeDescriptorSet{};
    VkDescriptorPool computeDescriptorPool{};
};


#endif //PIXELENGINE_PIXELCOMPUTEPIPELINE_H
