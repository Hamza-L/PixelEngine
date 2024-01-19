#include "PixelWindow.h"

PixelWindow::PixelWindow() :
	windowName("Default Window"),
	windowWidth(800),
	windowHeight(600)
{
	window = nullptr;
}

PixelWindow::~PixelWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void PixelWindow::initWindow(std::string wName, const int width, const int height)
{
    LOG(Level::INFO, "Initializing glfw window");
	windowName = wName;
	windowWidth = width;
	windowHeight = height;

	glfwInit();

	//set glfw to not work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(windowWidth, windowHeight, windowName.c_str(), nullptr, nullptr);

    int testWidth, testHeight;

    glfwGetFramebufferSize(window, &testWidth, &testHeight); //TODO: on Macos, the framebuffer size is somehow double the window size
}

GLFWwindow* PixelWindow::getWindow()
{
	return window;
}

bool PixelWindow::shouldClose()
{
	glfwPollEvents();
	return glfwWindowShouldClose(window);
}
