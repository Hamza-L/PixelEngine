//
// Created by hlahm on 2023-04-01.
//

#ifndef PIXELENGINE_PIXELGRAPHICSPIPELINE_H
#define PIXELENGINE_PIXELGRAPHICSPIPELINE_H

#include "PixelWindow.h"

#include <fstream>
#include <vector>

class PixelGraphicsPipeline {
public:
    explicit PixelGraphicsPipeline(VkDevice& device);
    PixelGraphicsPipeline(const PixelGraphicsPipeline&) = delete;
    void addVertexShader(const std::string& filename);
    void addFragmentShader(const std::string& filename);
    void createGraphicsPipeline(uint32_t width, uint32_t height);
    void createRenderPass();
    void cleanUp();
private:
    VkDevice& device;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkShaderModule vertexShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragmentShaderModule = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass renderPass;

    VkShaderModule addShaderModule(VkDevice device, const std::string& filename);
    std::vector<char> readFile(const std::string& filename);

};


#endif //PIXELENGINE_PIXELGRAPHICSPIPELINE_H
