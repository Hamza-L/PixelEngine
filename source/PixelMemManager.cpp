#include "PixelMemManager.h"

PixelMemManager* PixelMemManager::oneAndOnlyInstance = nullptr;
std::vector<PixelMemManager::PixelObjectDataHeader> PixelMemManager::metadataPerObject{};
std::vector<glm::vec4> PixelMemManager::listOfPositions{};
std::vector<glm::vec4> PixelMemManager::listOfNormals{};
std::vector<glm::vec4> PixelMemManager::listOfColors{};
std::vector<glm::vec4> PixelMemManager::listOfTexCoords{};
