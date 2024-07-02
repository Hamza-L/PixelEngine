#include "PixelRenderer.h"
#include "PixelRendererImpl.h"

PixelRendererImpl s_pixRendererImpl;
bool PixelRenderer::s_isInitialized = false;

ErrorLevel PixelRenderer::Initialize(){
    if (s_pixRendererImpl.initRenderer() == EXIT_FAILURE) {
        LOG_MSG(ErrorLevel::ERROR, "Error Initializing Pixel Renderer");
        return ErrorLevel::ERROR;
    } else {
        s_isInitialized = true;
        return ErrorLevel::OK;
    }
}

void PixelRenderer::Run(){
    if(!s_isInitialized){
        LOG_MSG(ErrorLevel::ERROR, "Cannot run renderer. It has not been initalized yet");
    }

    s_pixRendererImpl.run();
}

const Pixel::Devices PixelRenderer::GetDevices(){
    if(!s_isInitialized){
        LOG_MSG(ErrorLevel::ERROR, "Cannot retrieve renderer's Pixel::Devices. It has not been initalized yet");
        return {};
    }
    return s_pixRendererImpl.getDevices();
}
