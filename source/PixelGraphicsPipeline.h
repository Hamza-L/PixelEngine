//
// Created by hlahm on 2023-04-01.
//

#ifndef PIXELENGINE_PIXELGRAPHICSPIPELINE_H
#define PIXELENGINE_PIXELGRAPHICSPIPELINE_H

#include "PixelScene.h"
#include "PixelImage.h"

#include <fstream>
#include <vector>

class PixelGraphicsPipeline {
public:
    PixelGraphicsPipeline(VkDevice& device, VkExtent2D inputExtent);
    //PixelGraphicsPipeline(const PixelGraphicsPipeline&) = delete;
    void addVertexShader(const std::string& filename);
    void addFragmentShader(const std::string& filename);
    void populateGraphicsPipelineInfo();
    void populatePipelineLayout(PixelScene* scene);
    void createGraphicsPipeline(const VkRenderPass& inputRenderPass);
    void addRenderpassColorAttachment(PixelImage depthImage, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentStoreOp attachmentStoreOp, VkImageLayout attachmentReferenceLayout);
    void addRenderpassDepthAttachment(PixelImage depthImage);
    void createRenderPass();
    void cleanUp();

    VkRenderPass getRenderPass();
    VkPipeline getPipeline();
    VkPipelineLayout getPipelineLayout();
private:

    struct PixRenderpassAttachement
    {
        bool hasBeenDefined = false;
        //ATTACHMENTS
        //Color attachment for all of the renderpass (accessible to all subpass)
        VkAttachmentDescription attachmentDescription = {};

        //REFERENCES
        //attachment reference uses an attachment index to refer to the renderpass attachment list
        VkAttachmentReference attachmentReference = {};
    };

    //Graphics Pipeline parameters
    VkPipelineShaderStageCreateInfo vertexCreateShaderInfo = {};
    VkPipelineShaderStageCreateInfo fragmentCreateShaderInfo = {};
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    VkVertexInputBindingDescription inputBindingDescription{};
    std::array<VkVertexInputAttributeDescription, PixelObject::ATTRIBUTECOUNT> inputAttributeDescription{};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    VkViewport viewport = {};
    VkRect2D scissor{};
    VkExtent2D extent = {};
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    VkPipelineColorBlendAttachmentState blendAttachmentState = {};
    std::vector<VkDynamicState> dynamicstates = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};//allows us to use the comman vkcmdsetviewport & vkcmdsetscissor;
    VkPipelineColorBlendStateCreateInfo blendStateCreateInfo = {};

    //renderpass parameters
    std::vector<PixRenderpassAttachement> renderPassColorAttachments;
    PixRenderpassAttachement renderPassDepthAttachment = {}; //only one attachment can be used per renderpass/subpass

    VkDevice& device;
    VkRenderPass renderPass = VK_NULL_HANDLE;
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
