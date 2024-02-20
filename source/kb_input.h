//
// Created by hlahm on 2021-11-07.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
