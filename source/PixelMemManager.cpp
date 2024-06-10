#include "PixelMemManager.h"
#include <cstdlib>
#include "PixelConfig.h"
#include "PixelLogger.h"

// Memory Manager Singleton
class PixelMemManager {
  private:

    static std::vector<PixelObjectDataHeader> metadataPerObject;
    static std::vector<glm::vec4> listOfPositions;
    static std::vector<glm::vec4> listOfNormals;
    static std::vector<glm::vec4> listOfColors;
    static std::vector<glm::vec2> listOfTexCoords;

    UCHAR* m_imageData = nullptr;
    UCHAR* m_current_ptr = nullptr;
    bool m_isInit = false;

  public:
    PixelMemManager(){};

    ErrorLevel InitMemory(size_t sizeMemoryAllocation);
    ErrorLevel FreeAllMemory();
    UCHAR* GetMemoryChunk(const size_t bytesOfChunk);

    static glm::vec4 *getPositionsData(PixelObjectDataHeader dataHeader);
    static glm::vec4 *getNormalsData(PixelObjectDataHeader dataHeader);
    static glm::vec4 *getColorsData(PixelObjectDataHeader dataHeader);
    static glm::vec2 *getTexCoordsData(PixelObjectDataHeader dataHeader);

    bool IsMemoryInitialized(){return m_isInit;};

    static void addMeshObjectMemory(std::vector<glm::vec4> listOfPositions = {},
                                    std::vector<glm::vec4> listOfNormals = {},
                                    std::vector<glm::vec4> listOfColors = {},
                                    std::vector<glm::vec2> listOfTexCoords = {});
};

PixelMemManager* g_oneAndOnlyInstance = nullptr;

//TODO: move this to a large memory allocation rather than multiple large vector
std::vector<PixelObjectDataHeader> PixelMemManager::metadataPerObject{};
std::vector<glm::vec4> PixelMemManager::listOfPositions{};
std::vector<glm::vec4> PixelMemManager::listOfNormals{};
std::vector<glm::vec4> PixelMemManager::listOfColors{};
std::vector<glm::vec2> PixelMemManager::listOfTexCoords{};

ErrorLevel PixelMemManager::InitMemory(size_t sizeMemoryAllocation){
    LOG_MSG(ErrorLevel::INFO, "Initializing Memory");
    FreeAllMemory();
    m_imageData = (UCHAR*)std::malloc(sizeMemoryAllocation);
    m_current_ptr = m_imageData;
    m_isInit = true;
    return ErrorLevel::OK;
}

ErrorLevel PixelMemManager::FreeAllMemory(){
    LOG_MSG(ErrorLevel::INFO, "Freeing Memory");
    free(m_imageData);
    return ErrorLevel::OK;
}

UCHAR* PixelMemManager::GetMemoryChunk(const size_t bytesOfChunk){
    UCHAR* ptr_to_return = m_current_ptr;
    m_current_ptr += bytesOfChunk;
    if(m_current_ptr == ptr_to_return){
        printf("hello");
    }
    return ptr_to_return;
}

glm::vec4* PixelMemManager::getPositionsData(PixelObjectDataHeader dataHeader){
    glm::vec4* data_ptr = listOfPositions.data();
    data_ptr += dataHeader.positionOffset;
    return data_ptr;
}

glm::vec4* PixelMemManager::getNormalsData(PixelObjectDataHeader dataHeader){
    glm::vec4* data_ptr = listOfNormals.data();
    data_ptr += dataHeader.positionOffset;
    return data_ptr;
}

glm::vec4* PixelMemManager::getColorsData(PixelObjectDataHeader dataHeader){
    glm::vec4* data_ptr = listOfColors.data();
    data_ptr += dataHeader.positionOffset;
    return data_ptr;
}

glm::vec2* PixelMemManager::getTexCoordsData(PixelObjectDataHeader dataHeader){
    glm::vec2* data_ptr = listOfTexCoords.data();
    data_ptr += dataHeader.positionOffset;
    return data_ptr;
}

ErrorLevel PixelMemory::InitGlobalMemory(){
    if(!g_oneAndOnlyInstance) {
        g_oneAndOnlyInstance = new PixelMemManager();
        return g_oneAndOnlyInstance->InitMemory(START_MEMORY_ALLOCATION);
    } else {
        if(g_oneAndOnlyInstance->IsMemoryInitialized()){
            return ErrorLevel::OK;
        } else {
            return g_oneAndOnlyInstance->InitMemory(START_MEMORY_ALLOCATION);
        }
    }

}

ErrorLevel PixelMemory::FreeGlobalMemory(){
    if(!g_oneAndOnlyInstance){
        return ErrorLevel::OK;
    } else {
        return g_oneAndOnlyInstance->FreeAllMemory();
    }
}


UCHAR* PixelMemory::AllocateMemory(const size_t bytesToAllocate){
    return g_oneAndOnlyInstance->GetMemoryChunk(bytesToAllocate);
}
