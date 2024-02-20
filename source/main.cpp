
#include "glm/ext/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION

#include "kb_input.h"
#include "PixelLogger.h"
#include "PixelScene.h"
#include "PixelRenderer.h"


bool updateMyScene(PixelScene* pixScene){

    extern float scroll;
    PixelScene::UboVP newVP1{};
    newVP1.P = glm::perspective(glm::radians(45.0f + scroll), 960.0f/480.0f , 0.01f, 100.0f);
    newVP1.V = glm::lookAt(glm::vec3(5.0f, 5.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    newVP1.lightPos = glm::vec4(5.0f, 5.0f, 10.0f, 1.0f);

    pixScene->setSceneVP(newVP1);
    float tempScroll = scroll;

    // LOG(Level::DEBUG, "Scroll value is: %f", tempScroll);
    return true;
}

int main()
{
    Logger* log = Logger::get_instance();
    log->setSeverity(Level::DEBUG);

	LOG(Level::INFO, "main() entry point");

	PixelRenderer pixRenderer;

	if (pixRenderer.initRenderer() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

    PixelScene* mainScene = pixRenderer.createScene();
    mainScene->update = updateMyScene;
    // square.addTexture(computePipeline.getOutputTexture());
    // square.addTexture(computePipeline.getCustomTexture());

    // square.hide();

    // firstScene->addObject(object1);
    mainScene->addObject(std::make_shared<PixelObject>(PixelObject::Square()));

    pixRenderer.build(mainScene);
    pixRenderer.run();


	pixRenderer.cleanup();

	return 0;
}
