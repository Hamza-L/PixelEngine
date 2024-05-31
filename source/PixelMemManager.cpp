#include "PixelMemManager.h"

PixelMemManager* PixelMemManager::oneAndOnlyInstance = nullptr;
std::vector<PixelObjectDataHeader> PixelMemManager::metadataPerObject{};
std::vector<glm::vec4> PixelMemManager::listOfPositions{};
std::vector<glm::vec4> PixelMemManager::listOfNormals{};
std::vector<glm::vec4> PixelMemManager::listOfColors{};
std::vector<glm::vec2> PixelMemManager::listOfTexCoords{};

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
