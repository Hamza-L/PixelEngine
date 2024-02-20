//
// Created by hlahm on 2021-11-07.
//

#include "kb_input.h"
#include <cstdio>

const bool TEXTURE = true;
bool UP_PRESS = false;
bool E_KEY = false;
bool Q_KEY = false;
bool DOWN = false;
bool RIGHT = false;
bool LEFT = false;
bool SHIFT = false;
bool COM = false;
float scroll = 0;
bool ENTER = false;
bool ENTER_FLAG = true;
bool ESC = false;
bool MPRESS_R_Release = true;

//mouse events
bool MPRESS_R = false;
bool MFLAG_R = true;
bool MPRESS_L = false;
bool MRELEASE_L = false;
bool MCLICK_L = false;
bool MPRESS_M = false;
bool MFLAG_M = true;

//arcball parameter;
float FIT = 1.5f;
float GAIN = 2.0f;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {

    if (key == GLFW_KEY_W && action == GLFW_PRESS){
        UP_PRESS = true;
    } else if(key == GLFW_KEY_W && action == GLFW_RELEASE){
        UP_PRESS = false;
    }

	if (key == GLFW_KEY_E && action == GLFW_PRESS){
		E_KEY = true;
	} else if(key == GLFW_KEY_E && action == GLFW_RELEASE){
		E_KEY = false;
	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS){
		Q_KEY = true;
	} else if(key == GLFW_KEY_Q && action == GLFW_RELEASE){
		Q_KEY = false;
	}

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        ESC = true;
    } else if(key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE){
        ESC = false;
    }

    /*
    if (key == GLFW_KEY_W && action == GLFW_RELEASE){
        UP_PRESS_FLAG = true;
        DOWN_FLAG = true;
    }
     */

    if (key == GLFW_KEY_S && action == GLFW_PRESS){
        DOWN = true;
    } else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        DOWN = false;
    }/*
        if (key == GLFW_KEY_S && action == GLFW_RELEASE){
            UP_PRESS_FLAG = true;
            DOWN_FLAG = true;
        }*/

    if (key == GLFW_KEY_D && action == GLFW_PRESS){
        RIGHT = true;
    } else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        RIGHT = false;
    }/*
        if (key == GLFW_KEY_D && action == GLFW_RELEASE){
            LEFT_FLAG = true;
            RIGHT_FLAG = true;
        }*/

    if (key == GLFW_KEY_A && action == GLFW_PRESS){
        LEFT = true;
    } else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        LEFT = false;
    }/*
        if (key == GLFW_KEY_A && action == GLFW_RELEASE){
            LEFT_FLAG = true;
            RIGHT_FLAG = true;
        }*/

    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS){
        SHIFT = true;
    } else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
        SHIFT = false;
    }

    if (key == GLFW_KEY_LEFT_SUPER && action == GLFW_PRESS){
        COM = true;
    } else if (key == GLFW_KEY_LEFT_SUPER && action == GLFW_RELEASE) {
        COM = false;
    }

    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS){
        ENTER = true;
    }
    if (key == GLFW_KEY_ENTER && action == GLFW_RELEASE) {
        ENTER = false;
        ENTER_FLAG = true;
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods) {

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		MPRESS_L = true;
		MCLICK_L = true;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
		MRELEASE_L = true;
		MPRESS_L = false;
		MCLICK_L = false;
	} else {
        MPRESS_L = false;
		MCLICK_L = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        MPRESS_R = true;
    } else {
        MPRESS_R = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
        MFLAG_R = true;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS){
        MPRESS_M = true;
    } else {
        MPRESS_M = false;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE){
        MFLAG_M = true;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    //std::cout<<yoffset<<std::endl;
    scroll += 2.0f*yoffset;
}
