#ifndef ANIMATEDPIXELIMAGE_H_
#define ANIMATEDPIXELIMAGE_H_

#include "VKWPixelImage.h"
#include "glm/glm.hpp"

class AnimatedPixelImage {
  public:
    AnimatedPixelImage() = default;

    void LoadFlipBookImageFile(const char *filePath);
    void Update();
    std::vector<glm::vec2> GetCurrentAnimationClip();

  private:
    VKWPixelImage m_pixelImage{};

    UINT32 m_currentAnimationFrame = 0;
    UINT32 m_MaxAnimationFrames = 0;
    std::vector<UINT32> m_maxAnimationFramesPerAnimation = {};
};

#endif // ANIMATEDPIXELIMAGE_H_
