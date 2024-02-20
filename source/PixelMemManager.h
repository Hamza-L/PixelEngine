#ifndef PIXELMEMMANAGER_H_
#define PIXELMEMMANAGER_H_

#include <vector>
#include "glm/glm.hpp"

// Memory Manager Singleton
class PixelMemManager {
  private:
    PixelMemManager(){};

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

    static PixelMemManager *oneAndOnlyInstance;
    static std::vector<PixelObjectDataHeader> metadataPerObject;
    static std::vector<glm::vec4> listOfPositions;
    static std::vector<glm::vec4> listOfNormals;
    static std::vector<glm::vec4> listOfColors;
    static std::vector<glm::vec4> listOfTexCoords;

  public:
    static PixelMemManager *get_instance() {
        if (oneAndOnlyInstance == nullptr) {
            oneAndOnlyInstance = new PixelMemManager();
        }
        return oneAndOnlyInstance;
    }



};

#endif // PIXELMEMMANAGER_H_
