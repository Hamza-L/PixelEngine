
#include "glm/ext/matrix_transform.hpp"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION

#include "PixelLogger.h"
#include "PixelRenderer.h"
#include "PixelScene.h"
#include "kb_input.h"

constexpr UINT16 WIDTH = 960;
constexpr UINT16 HEIGHT = 480;

inline bool updateMyScene(PixelScene *pixScene) {

    extern float scroll;
    extern bool LEFT;
    extern bool RIGHT;
    extern bool UP;
    extern bool DOWN;
    extern bool SHIFT;

    PixelScene::UboVP newVP1{};
    glm::mat4 newTransform = {pixScene->getObjectAt(0)->getPushObj()->M};
    float speed = 0.001 * (SHIFT ? 3.f : 1.f);
    if(RIGHT){
        newTransform[3][0] += speed;
    }
    if(LEFT){
        newTransform[3][0] -= speed;
    }
    if(DOWN){
        newTransform[3][2] += speed;
    }
    if(UP){
        newTransform[3][2] -= speed;
    }

    pixScene->getObjectAt(0)->setTransform(newTransform);

    glm::vec3 playerPosition = getPositionFromMatrix(pixScene->getObjectAt(0)->getPushObj()->M);
    // LOG_MSG(ErrorLevel::INFO, "Position : {%f,%f,%f}", playerPosition.x, playerPosition.y, playerPosition.z);
    // newVP1.P = glm::perspective(glm::radians(45.0f + scroll), 960.0f / 480.0f, 0.01f, 100.0f);

    float widthVal = (float)WIDTH * (scroll+20) * 0.001f;
    float heightVal = (float)HEIGHT * (scroll+20) * 0.001f;
    newVP1.P = glm::ortho(-(widthVal / 2.0f), widthVal / 2.0f,
                          -heightVal / 2.0f, (heightVal / 2.0f),
                          -1000.0f, 100.0f);

    newVP1.V = glm::lookAt(glm::vec3(5.0f, 5.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    newVP1.lightPos = glm::vec4(1000.0f, 1000.0f, 1000.0f, 0.0f);

    pixScene->setSceneVP(newVP1);
    float tempScroll = scroll;


    // LOG(Level::DEBUG, "Scroll value is: %f", tempScroll);
    return true;
}

int main() {
    Logger *log = Logger::get_instance();
    log->setSeverity(ErrorLevel::DEBUG);

    LOG_SCOPED(ErrorLevel::INFO, "main() entry point");

    PixelRenderer pixRenderer;
    PixelMemory::InitGlobalMemory();

    if (pixRenderer.initRenderer() == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    PixelScene *mainScene = pixRenderer.createScene();
    mainScene->update = updateMyScene;
    // square.addTexture(computePipeline.getOutputTexture());
    // square.addTexture(computePipeline.getCustomTexture());

    // square.hide();

    // firstScene->addObject(object1);
    PixelObject square = PixelObject::Square();
    PixelObject ground = PixelObject::Square();

    square.addTexture(pixRenderer.getDevices(), "assets/ManAnimation.png");
    mainScene->addObject(std::make_shared<PixelObject>(square));
    // mainScene->addObject(std::make_shared<PixelObject>(ground));

    pixRenderer.build(mainScene);
    pixRenderer.run();

    pixRenderer.cleanup();

    PixelMemory::FreeGlobalMemory();

    return 0;
}
