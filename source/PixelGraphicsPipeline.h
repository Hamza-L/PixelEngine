//
// Created by hlahm on 2023-04-01.
//

#ifndef PIXELENGINE_PIXELGRAPHICSPIPELINE_H
#define PIXELENGINE_PIXELGRAPHICSPIPELINE_H

#include "PixelWindow.h"
#include "PixelObject.h"

#include <fstream>
#include <vector>

class PixelGraphicsPipeline {
public:
    explicit PixelGraphicsPipeline(VkDevice& device, VkExtent2D inputExtent, VkFormat inputFormat);
    PixelGraphicsPipeline(const PixelGraphicsPipeline&) = delete;
    void addVertexShader(const std::string& filename);
    void addFragmentShader(const std::string& filename);
    void populateGraphicsPipelineInfo();
    void createGraphicsPipeline(VkRenderPass inputRenderPass);
    void createRenderPass();
    void cleanUp();

    VkRenderPass getRenderPass();
    VkPipeline getPipeline();

    VkPipelineShaderStageCreateInfo vertexCreateShaderInfo = {};
    VkPipelineShaderStageCreateInfo fragmentCreateShaderInfo = {};
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    VkViewport viewport = {};
    VkExtent2D extent = {};
    VkFormat format = {};
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    VkPipelineColorBlendAttachmentState blendAttachmentState = {};
    std::vector<VkDynamicState> dynamicstates = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};//allows us to use the comman vkcmdsetviewport & vkcmdsetscissor;
    VkPipelineColorBlendStateCreateInfo blendStateCreateInfo = {};
private:
    VkDevice& device;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkShaderModule vertexShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragmentShaderModule = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};


    VkShaderModule addShaderModule(const std::string& filename);
    static std::vector<char> readFile(const std::string& filename);
    bool wasRenderPassCreated = false;

};


#endif //PIXELENGINE_PIXELGRAPHICSPIPELINE_H
