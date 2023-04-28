//
// Created by hlahm on 2023-04-01.
//

#include "PixelGraphicsPipeline.h"

#include <fstream>
#include <array>

std::vector<char> PixelGraphicsPipeline::readFile(const std::string& filename) {

    //open stream from given file
    //binary to read file as a binary file (sprv)
    //ate tells to read from end of file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    //check if we found the file
    if(!file.is_open())
    {
        throw std::runtime_error("failed to open a file");
    }

    size_t filesize = (size_t)file.tellg();

    std::vector<char> outputBuffer(filesize);

    file.seekg(0);
    file.read(outputBuffer.data(), filesize);

    file.close();

    return outputBuffer;
}

VkShaderModule PixelGraphicsPipeline::addShaderModule(const std::string &filename) {

    std::vector<char> code = readFile(filename);

    VkShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = code.size();
    shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &shaderModule);
    if(result != VK_SUCCESS)
    {
        std::string outMessage = "failed to create shader module from file: %s\n";
        outMessage.append(filename);
        throw std::runtime_error(outMessage);
    }

    return shaderModule;
}

void PixelGraphicsPipeline::addVertexShader(const std::string &filename) {
    vertexShaderModule = addShaderModule(filename);
}

void PixelGraphicsPipeline::addFragmentShader(const std::string &filename) {
    fragmentShaderModule = addShaderModule(filename);
}

void PixelGraphicsPipeline::createGraphicsPipeline(const VkRenderPass& inputRenderPass) {

    //the shader create infos have to be passed in as an array
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexCreateShaderInfo, fragmentCreateShaderInfo};

    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    //Depth stencil testing
    //TODO:Setup Depth Stencil

    //create renderpass
    if(inputRenderPass == nullptr)
    {
        createRenderPass();
    } else
    {
        renderPass = inputRenderPass;
    }


    //graphics pipeline info
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2; //vertex and fragment stage
    graphicsPipelineCreateInfo.pStages = shaderStages; //list of shader stages
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo; //all fixed functions pipeline stages
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = VK_NULL_HANDLE; //&dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &blendStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
    graphicsPipelineCreateInfo.layout = pipelineLayout; //pipeline layout this pipeline should use
    graphicsPipelineCreateInfo.renderPass = renderPass; //render pass description the pipeline is compatible with
    graphicsPipelineCreateInfo.subpass = 0; //one subpass per pipeline

    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; //existing pipeline to derive from
    graphicsPipelineCreateInfo.basePipelineIndex = -1; //or index pipeline from of multiple pipelines created once suing specfic funciton

    //create graphics pipeline
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline");
    }

    vkDestroyShaderModule(device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
}

void PixelGraphicsPipeline::cleanUp() {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    if(renderPass != VK_NULL_HANDLE && wasRenderPassCreated)
    vkDestroyRenderPass(device, renderPass, nullptr);
}

void PixelGraphicsPipeline::createRenderPass() {

    //ATTACHMENTS
    //Color attachment for all the renderpass (accessible to all subpass)
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //this clears the buffer when we start the renderpass
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //we want to present the result so we keep it
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //renderpass (before the subpasses) layout
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //renderpass (after the subpasses) layout

    // depth attachment for all the renderpass (accessible to all subpass)
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = format; //need to use chooseformat function from pixelrenderer
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //this clears the buffer when we start the renderpass
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //we want to present the result so we keep it
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //renderpass (before the subpasses) layout
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //renderpass (after the subpasses) layout


    //REFERENCES
    //attachment reference uses an attachment index to refer to the renderpass attachment list
    VkAttachmentReference attachmentReference = {};
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //info about our first subpass
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;

    //dependencies
    std::array<VkSubpassDependency, 2> subpassDependencies = {};
    //conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    //SUBPASS MUST HAPPEN AFTER THE FOLLOWING
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; //anything that takes place outside of the subpass
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; //the end of the pipeline. This stage has to happen first before we proceed to this subpass
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //SUBPASS MUST HAPPEN BEFORE THE FOLLOWING
    subpassDependencies[0].dstSubpass = 0; //the first subpass in the list we sent
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //this stage has to happen after this subpass;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = 0; //no dependency flags

    //conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    //SUBPASS MUST HAPPEN AFTER THE FOLLOWING
    subpassDependencies[1].srcSubpass = 0; //anything that takes place outside of the subpass
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; //the end of the pipeline. This stage has to happen first before we proceed to this subpass
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //SUBPASS MUST HAPPEN BEFORE THE FOLLOWING
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL; //the first subpass in the list we sent
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; //this stage has to happen after this subpass;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = 0; //no dependency flags

    //create info for renderpass
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderPassCreateInfo.pDependencies = subpassDependencies.data();

    VkResult result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create renderpass");
    } else
    {
        wasRenderPassCreated = true;
    }
}

PixelGraphicsPipeline::PixelGraphicsPipeline(VkDevice& device, VkExtent2D inputExtent, VkFormat inputFormat) : device(device), extent(inputExtent), format(inputFormat) {

}

VkRenderPass PixelGraphicsPipeline::getRenderPass() {
    return renderPass;
}

void PixelGraphicsPipeline::populateGraphicsPipelineInfo() {
//Vertex-Stage creation
    vertexCreateShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexCreateShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexCreateShaderInfo.module = vertexShaderModule;
    vertexCreateShaderInfo.pName = "main"; //the entry point of the shader

    //Vertex-Stage creation
    fragmentCreateShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentCreateShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentCreateShaderInfo.module = fragmentShaderModule;
    fragmentCreateShaderInfo.pName = "main"; //the entry point of the shader

    //vertex input info-----------
    //How data for a single vertex is laid out

    inputBindingDescription.binding = 0;
    inputBindingDescription.stride = sizeof(PixelObject::Vertex);
    inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //how to move between data after each vertex
                                                                     //VK_VERTEX_INPUT_RATE_VERTEX : Move on to the next vertex
                                                                     //VK_VERTEX_INPUT_RATE_INSTANCE : Move on to the next instance

    //How the data within a vertex is descripted
    //const int numOfVertexAttribute = PixelObject::getNumofAttributes();
    //std::array<VkVertexInputAttributeDescription, numOfVertexAttribute> inputAttributeDescription{};

    //fills in each Vertex Input Attribute Description struct for each attributes in the Vertex Object (position, color etc...)
    for(size_t i = 0; i < PixelObject::getNumofAttributes(); i++)
    {
        inputAttributeDescription[i].binding = 0; //matches the layout(binding = 0)
        inputAttributeDescription[i].location = static_cast<uint32_t>(i); //matches the layout(location = 0)
        inputAttributeDescription[i].format = VK_FORMAT_R32G32B32A32_SFLOAT; //the format of the attribute (vec3)
        inputAttributeDescription[i].offset = static_cast<uint32_t>(i * 16); //each vec4 has 16 bytes. so the offset into the struct shifts by 16 bytes per vec4
    }

    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &inputBindingDescription; //list of binding description info (spacing, stride etc,,,)
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(PixelObject::getNumofAttributes());
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescription.data(); //list of attribute description (data format and where to bind to/from)

    //Input Assembly
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = false; // allow to restart list of triangle fans

    //viewport and scissors
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    scissor.offset = {0,0};
    scissor.extent = extent;
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    //dynamic state
    /*
    //point to somethings you may change dynamically
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicstates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicstates.data();
     */

    //rasterizer
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE; //change if fragments beyond the far plane are clipped to plane. requires GPU Feature
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; //suitable for pipeline without framebuffer
    rasterizationStateCreateInfo.lineWidth = 1.0f; //need gpu feature for anything else then 1
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;


    //multisampling
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    //blending
    blendAttachmentState.colorWriteMask =   VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    //blending uses the following equation
    //----- (srcColorBlendFactor * newColor) colorBlendOp (dstColorBlendFactor * oldColor) = result

    blendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendStateCreateInfo.logicOpEnable = VK_FALSE; //alternative to calculation is to use logic op
    blendStateCreateInfo.attachmentCount = 1;
    blendStateCreateInfo.pAttachments = &blendAttachmentState;

    //pipeline layout
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
}

VkPipeline PixelGraphicsPipeline::getPipeline() {
    return graphicsPipeline;
}

void PixelGraphicsPipeline::populatePipelineLayout(PixelScene* scene) {
    //pipeline layout
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = scene->getDescriptorSetLayout();
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &PixelObject::pushConstantRange;
}

VkPipelineLayout PixelGraphicsPipeline::getPipelineLayout() {
    return pipelineLayout;
}


