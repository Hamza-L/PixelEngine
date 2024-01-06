//
// Created by hlahm on 2023-05-31.
//

#include "PixelComputePipeline.h"

#include <array>

void PixelComputePipeline::addComputeShader(PixBackend* devices, const std::string &filename) {
    computeShaderModule = addShaderModule(devices->logicalDevice, filename);

    computeCreateShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeCreateShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeCreateShaderInfo.module = computeShaderModule;
    computeCreateShaderInfo.pName = "main"; //the entry point of the shader
}

void PixelComputePipeline::cleanUp(PixBackend* devices) {


    if(!raytracedInputTexture.hasBeenCleaned())
    {
        raytracedInputTexture.cleanUp(devices);
    }

    if(!customTexture.hasBeenCleaned())
    {
        customTexture.cleanUp(devices);
    }

    if(!raytracedOutputTexture.hasBeenCleaned())
    {
        raytracedOutputTexture.cleanUp(devices);
    }

    vkDestroyPipeline(devices->logicalDevice, computePipeline, nullptr);
    vkDestroyPipelineLayout(devices->logicalDevice, computePipelineLayout, nullptr);

    vkDestroyDescriptorPool(devices->logicalDevice, computeDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(devices->logicalDevice, computeDescriptorSetLayout, nullptr);
}

void PixelComputePipeline::populatePipelineLayout() {
    //pipeline layout
    computePipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //computePipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(scene->getAllDescriptorSetLayouts()->size());
    //computePipelineLayoutCreateInfo.pSetLayouts = scene->getAllDescriptorSetLayouts()->data();
    computePipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    computePipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
}

void PixelComputePipeline::initImageBufferStorage(PixBackend* devices) {
    uint32_t width = 1024;
    uint32_t height = 768;
    raytracedInputTexture = PixelImage(width, height, false);
    raytracedInputTexture.loadEmptyTexture(devices, width, height, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    raytracedOutputTexture = PixelImage(width, height, false);
    raytracedOutputTexture.loadEmptyTexture(devices, width, height, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    customTexture = PixelImage(width, height, false);
    customTexture.loadEmptyTexture(devices, width, height, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
}

void PixelComputePipeline::init(PixBackend* devices) {
    addComputeShader(devices, "shaders/comp.spv");
    initImageBufferStorage(devices);
    createDescriptorSetLayout(devices);
    createDescriptorPool(devices);
    createDescriptorSets(devices);
    createComputePipelineLayout(devices);
    createComputePipeline(devices);
}

void PixelComputePipeline::createDescriptorSetLayout(PixBackend* devices) {
    std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};

    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[0].pImmutableSamplers = nullptr;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[1].pImmutableSamplers = nullptr;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layoutBindings[2].pImmutableSamplers = nullptr;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(devices->logicalDevice, &layoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute descriptor set layout!");
    }
}

void PixelComputePipeline::createDescriptorSets(PixBackend* devices) {

    //allocate info for texture descriptor set. they are not created but allocated from the pool
    VkDescriptorSetAllocateInfo textureSetAllocateInfo{};
    textureSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    textureSetAllocateInfo.descriptorPool = computeDescriptorPool;
    textureSetAllocateInfo.descriptorSetCount = 1;
    textureSetAllocateInfo.pSetLayouts = &computeDescriptorSetLayout;
    // has to be 1:1 relationship with descriptor sets

    VkResult result = vkAllocateDescriptorSets(devices->logicalDevice, &textureSetAllocateInfo, &computeDescriptorSet);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor set for compute textures");
    }

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    VkDescriptorImageInfo inputImageBuffer{};
    inputImageBuffer.imageView = raytracedInputTexture.getImageView();
    inputImageBuffer.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    //inputImageBuffer.sampler = VK_NULL_HANDLE;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = computeDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &inputImageBuffer;

    VkDescriptorImageInfo ouputImageBuffer{};
    ouputImageBuffer.imageView = raytracedOutputTexture.getImageView();
    ouputImageBuffer.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    //ouputImageBuffer.sampler = VK_NULL_HANDLE;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = computeDescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &ouputImageBuffer;

    VkDescriptorImageInfo customImageBuffer{};
    customImageBuffer.imageView = customTexture.getImageView();
    customImageBuffer.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    //ouputImageBuffer.sampler = VK_NULL_HANDLE;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = computeDescriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &customImageBuffer;

    vkUpdateDescriptorSets(devices->logicalDevice, 3, descriptorWrites.data(), 0, nullptr);
}

void PixelComputePipeline::createDescriptorPool(PixBackend* devices) {

    uint32_t MAX_DESCRIPTOR_SETS = 1000;
    uint32_t MAX_DESCRIPTOR_COUNT = 500;

    //number of descriptors and not descriptor sets. combined, it makes the pool size
    VkDescriptorPoolSize uniformDescriptorSize{};
    uniformDescriptorSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformDescriptorSize.descriptorCount = static_cast<uint32_t>(MAX_DESCRIPTOR_COUNT); //one descriptor per swapchain image

    VkDescriptorPoolSize storageBufferDescriptorSize{};
    storageBufferDescriptorSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageBufferDescriptorSize.descriptorCount = static_cast<uint32_t>(MAX_DESCRIPTOR_COUNT); //one descriptor per swapchain image

    VkDescriptorPoolSize imageStorageDescriptorSize{};
    imageStorageDescriptorSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imageStorageDescriptorSize.descriptorCount = static_cast<uint32_t>(MAX_DESCRIPTOR_COUNT); //one descriptor per swapchain image

    std::array<VkDescriptorPoolSize, 3> poolSizes = {uniformDescriptorSize, storageBufferDescriptorSize, imageStorageDescriptorSize};

    //includes info about the descriptor set that contains the descriptor
    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = static_cast<uint32_t>(MAX_DESCRIPTOR_SETS); //maximum number of descriptor sets that can be created from pool
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolCreateInfo.pPoolSizes = poolSizes.data();

    VkResult result = vkCreateDescriptorPool(devices->logicalDevice, &poolCreateInfo, nullptr, &computeDescriptorPool);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool for compute pipeline");
    }


}

void PixelComputePipeline::createComputePipeline(PixBackend* devices) {
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = computePipelineLayout;
    pipelineInfo.stage = computeCreateShaderInfo;

    VkResult result = vkCreateComputePipelines(devices->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create the compute pipeline");
    }

    //we no longer need it once the pipeline has been created
    vkDestroyShaderModule(devices->logicalDevice, computeShaderModule, nullptr);
}

void PixelComputePipeline::createComputePipelineLayout(PixBackend* devices) {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &PixelComputePipeline::pushComputeConstantRange;

    if (vkCreatePipelineLayout(devices->logicalDevice, &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }
}

VkPipeline PixelComputePipeline::getPipeline() {
    return computePipeline;
}

VkPipelineLayout PixelComputePipeline::getPipelineLayout() {
    return computePipelineLayout;
}

VkDescriptorSet PixelComputePipeline::getDescriptorSet() {
    return computeDescriptorSet;
}

PixelImage* PixelComputePipeline::getInputTexture() {
    return &raytracedInputTexture;
}

PixelImage* PixelComputePipeline::getCustomTexture() {
    return &customTexture;
}

PixelImage* PixelComputePipeline::getOutputTexture() {
    return &raytracedOutputTexture;
}


