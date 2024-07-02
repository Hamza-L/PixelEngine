//
// Created by Zara Hussain on 2023-04-20.
//

#ifndef PIXELENGINE_PIXELSCENE_H
#define PIXELENGINE_PIXELSCENE_H

#define GLM_ENABLE_EXPERIMENTAL

#include "PixelLogger.h"
#include "PixelObject.h"
#include "Renderable.h"

static const glm::mat4 MAT4_IDENTITY = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

const int MAX_OBJECTS = 10;
const int MAX_TEXTURE_PER_OBJECT = 16;
enum DescSetLayoutIndex { UBOS, TEXTURES };

class PixelScene : public Renderable {
  public:
    PixelScene();
    // PixelScene(const PixelScene&) = delete;

    void Build();
    void Render();

    // update function
    bool (*update)(PixelScene *pixScene) = nullptr;

    // setter functions
    void addObject(std::shared_ptr<PixelObject> pixObject);

    // getter functions
    VkDescriptorPool *getDescriptorPool();
    std::vector<VkDescriptorSetLayout> *getAllDescriptorSetLayouts();
    VkDescriptorSetLayout *getDescriptorSetLayout(DescSetLayoutIndex indx);
    std::vector<VkDescriptorSet> *getUniformDescriptorSets();
    VkDescriptorSet *getUniformDescriptorSetAt(int index);
    VkDescriptorSet *getTextureDescriptorSet();
    static VkDeviceSize getUniformBufferSize();
    VkDeviceSize getDynamicUniformBufferSize() const;
    VkDeviceSize getMinAlignment() const;
    VkBuffer *getUniformBuffers(int index);
    VkDeviceMemory *getUniformBufferMemories(int index);
    VkBuffer *getDynamicUniformBuffers(int index);
    VkDeviceMemory *getDynamicUniformBufferMemories(int index);
    int getNumObjects();
    unsigned int getSceneID() { return m_sceneID; }
    std::shared_ptr<PixelObject> getObjectAt(int index);
    std::vector<VKWPixelImage> getAllTextures();
    Pixel::UboVP getSceneVP();
    glm::vec3 getCameraPos();
    glm::vec3 getLookAtVec();
    std::string getName();

    // setter functions
    void setSceneName(const char *name);
    void setSceneVP(Pixel::UboVP vpData);
    void setSceneV(glm::mat4 V);
    void setSceneP(glm::mat4 P);
    void setCameraPos(glm::vec3 camPos);
    void setLookAtPos(glm::vec3 lookAtPos);
    void setSceneID(unsigned int sceneID) { m_sceneID = sceneID; }

    // create functions
    void createDescriptorSetLayout(const Pixel::Devices& devices);

    // update functons
    void updateUniformBuffer(const Pixel::Devices& devices, uint32_t bufferIndex);
    void updateDynamicUniformBuffer(const Pixel::Devices& devices, uint32_t bufferIndex);

    // helper functions
    void initialize(const Pixel::Devices& devices);
    void resizeBuffers(size_t newSize);
    void resizeDesciptorSets(size_t newSize);
    static bool areMatricesEqual(glm::mat4 x, glm::mat4 y);

    // cleanup
    void cleanup(const Pixel::Devices& devices);

  private:
    std::string m_sceneName{};
    unsigned int m_sceneID{};

    // objects
    std::vector<std::shared_ptr<PixelObject>> m_allObjects{};
    std::vector<Pixel::Vertex> allVertices{};
    std::vector<uint32_t> allIndices{};

    glm::vec3 m_lookAtVec{};
    glm::vec3 m_cameraPos{};

    // helper functions
    void getMinUBOOffset(VkPhysicalDevice physicalDevice);

    // allocator functions
    void allocateDynamicBufferTransferSpace();

    //------UNIFORM BUFFER
    Pixel::UboVP sceneVP; // model view projection matrix
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemories;
    std::vector<VkBuffer> dynamicUniformBuffers;
    std::vector<VkDeviceMemory> dynamicUniformBufferMemories;
    Pixel::DynamicUBObj *modelTransferSpace{};

    //------TEXTURES

    // utility (Dynamic Buffers)
    VkDeviceSize minUBOOffset{};  // this is device specific and is a constant
    size_t objectUBOAllignment{}; // this is a multiple of minUBOOffset and will depend on the size of the object's UBO
    std::vector<bool> buffersUpdated;

    // vulkan component
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_uniformDescriptorSets{};
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts{};
    VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;
};

#endif // PIXELENGINE_PIXELSCENE_H
