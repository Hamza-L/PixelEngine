//
// Created by Zara Hussain on 2023-04-14.
//

#ifndef PIXELENGINE_PIXELOBJECT_H
#define PIXELENGINE_PIXELOBJECT_H

#include "VKWPixelImage.h"
#include "AnimatedPixelImage.h"
#include "Utility.h"
#include "PixelMemManager.h"

#include <string>
#include <vector>

class PixelObject {
public:

    PixelObject(std::vector<Pixel::Vertex> vertices, std::vector<uint32_t> indices);
    PixelObject(std::string filename);
    PixelObject() = default;

    //getters
    int getVertexCount();
    std::vector<Pixel::Vertex>* getVertices();
    VkDeviceSize getVertexBufferSize();
    VkBuffer* getVertexBuffer();
    VkDeviceMemory* getVertexBufferMemory();
    int getIndexCount();
    std::vector<uint32_t>* getIndices();
    VkDeviceSize getIndexBufferSize();
    VkBuffer* getIndexBuffer();
    VkDeviceMemory* getIndexBufferMemory();
    Pixel::PObj* getPushObj();
    Pixel::DynamicUBObj* getDynamicUBObj();
    std::vector<VKWPixelImage> getTextures(){return m_textures;}
    int getGraphicsPipelineIndex(){return graphicsPipelineIndex;};
    static constexpr VkPushConstantRange pushConstantRange {VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Pixel::PObj)};

    //setters
    void setDynamicUBObj(Pixel::DynamicUBObj pushObjData);
    void setTexID(int texID){dynamicUBO.texIndex = texID;};
    void setPushObj(Pixel::PObj pushObjData);
    void setGraphicsPipelineIndex(int pipelineIndx){graphicsPipelineIndex = pipelineIndx;};

    //cleanup
    void cleanup(const Pixel::Devices* device);

    // update
    void Update();


    //helper functions
    //returns the number of members of the Pixel::Vertex Struct
    void importObjFile(const std::string& filename);
    void setGenericColor(glm::vec4 color);
    void addTransform(glm::mat4 matTransform);
    void setTransform(glm::mat4 matTransform);
    void addTexture(Pixel::Devices* devices, const char* textureFile);
    void addTexture(VKWPixelImage* pixImage);
    void setTextureIDOffset(int offset){texIDOffset = offset;};
    void hide(){m_isHidden = true;};
    void unhide(){m_isHidden = false;};
    bool isHidden(){return m_isHidden;};

    //make shapes
    static PixelObject Square();
    static PixelObject Cube();

    // cached shape (to avoid importing for every shape)
    bool cubeCached = false;


private:
    //member variables
    std::vector<Pixel::Vertex> m_vertices{};
    std::vector<uint32_t> m_indices{};
    std::string name{};
    bool m_isHidden = false;

    //transforms
    Pixel::DynamicUBObj dynamicUBO = {};
    Pixel::PObj pushObj = {glm::mat4(1.0f)};

    //vulkan components
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

    //texture used
    std::vector<VKWPixelImage> m_textures;
    int texIDOffset = 0;

    //current Animated texture;
    AnimatedPixelImage m_animatedtexture;

    //pipeline used
    int graphicsPipelineIndex = 0;
};


#endif //PIXELENGINE_PIXELOBJECT_H
