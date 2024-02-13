
#include "glm/ext/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION

#include "PixelScene.h"
#include "PixelRenderer.h"

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

    std::vector<PixelObject::Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 0
        {{1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},  // 1
        {{1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},   // 2
        {{-1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}   // 3
    };
    std::vector<uint32_t> indices{1, 2, 0, 2, 3, 0};

    auto square = std::make_shared<PixelObject>(vertices, indices);

    // square->addTexture(&mainDevice, "Skull.jpg");
    square->setGraphicsPipelineIndex(0);
    // square.addTexture(computePipeline.getOutputTexture());
    // square.addTexture(computePipeline.getCustomTexture());

    // square.hide();

    // firstScene->addObject(object1);
    mainScene->addObject(square);

    PixelScene::UboVP newVP1{};
    newVP1.P = glm::perspective(glm::radians(45.0f), 960.0f/480.0f , 0.01f, 100.0f);
    newVP1.V = glm::lookAt(glm::vec3(5.0f, 5.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    newVP1.lightPos = glm::vec4(0.0f, 5.0f, 25.0f, 1.0f);

    mainScene->setSceneVP(newVP1);



    pixRenderer.build(mainScene);
    pixRenderer.run();


	pixRenderer.cleanup();

	return 0;
}
