
#define STB_IMAGE_IMPLEMENTATION

#include "PixelScene.h"
#include "PixelRenderer.h"

int main()
{

	PixelRenderer pixRenderer;

	if (pixRenderer.initRenderer() == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}

	//PixelScene scene;
	//pixRenderer.build(&scene) //creates the whole backend appropriate for the scene (ie connects the scene to the renderer)

    pixRenderer.run();


	pixRenderer.cleanup();

	return 0;
}
