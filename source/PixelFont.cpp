#include "PixelFont.h"
#include "PixelLogger.h"
FT_Library  g_Fontlibrary;

bool PixelFont::isInitialized = false;
std::unordered_map<std::string, FT_Face> PixelFont::s_fontFaces = {};

void PixelFont::loadFont(const char *fileName, int *fileSize) {
    FT_Error err = false;
    if(!isInitialized){
        err = FT_Init_FreeType( &g_Fontlibrary );
        if ( err ){
            LOG_MSG(ErrorLevel::ERROR, "Error whilst initializing FreeType Library");
        } else {
            isInitialized = true;
        }
    }

    FT_Face newFace;
    err = FT_New_Face( g_Fontlibrary, fileName, 0, &newFace );
    if ( err == FT_Err_Unknown_File_Format){
        LOG_MSG(ErrorLevel::ERROR, "Font format is unsupported");
    } else if ( err ){
        LOG_MSG(ErrorLevel::ERROR, "Font could not be loaded");
    } else {
        std::string key = fileName;
        s_fontFaces[key] = newFace;
    }

    return;
}
