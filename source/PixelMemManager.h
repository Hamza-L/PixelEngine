#ifndef PIXELMEMMANAGER_H_
#define PIXELMEMMANAGER_H_

#include <vector>
#include "glm/glm.hpp"

struct PixelObjectDataHeader {
    unsigned int id;
    int positionOffset;
    int positionSize;
    int normalOffset;
    int normalSize;
    int colorOffset;
    int colorSize;
    int texCoordOffset;
    int texCoordSize;
};

// Memory Manager Singleton
class PixelMemManager {
  private:
    PixelMemManager(){};

    static PixelMemManager *oneAndOnlyInstance;
    static std::vector<PixelObjectDataHeader> metadataPerObject;
    static std::vector<glm::vec4> listOfPositions;
    static std::vector<glm::vec4> listOfNormals;
    static std::vector<glm::vec4> listOfColors;
    static std::vector<glm::vec2> listOfTexCoords;

  public:
    static PixelMemManager* get_instance() {
        if (oneAndOnlyInstance == nullptr) {
            oneAndOnlyInstance = new PixelMemManager();
        }
        return oneAndOnlyInstance;
    }

    static glm::vec4 *getPositionsData(PixelObjectDataHeader dataHeader);
    static glm::vec4 *getNormalsData(PixelObjectDataHeader dataHeader);
    static glm::vec4 *getColorsData(PixelObjectDataHeader dataHeader);
    static glm::vec2 *getTexCoordsData(PixelObjectDataHeader dataHeader);

    static void addMeshObjectMemory(std::vector<glm::vec4> listOfPositions = {},
                                    std::vector<glm::vec4> listOfNormals = {},
                                    std::vector<glm::vec4> listOfColors = {},
                                    std::vector<glm::vec2> listOfTexCoords = {});
};

#endif // PIXELMEMMANAGER_H_
