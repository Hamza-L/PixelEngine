#include "PixelFont.h"
#include <cstdio>
#include <cstdlib>
#include <string.h>

#define READ_BE16(mem) ((((UCHAR*)(mem))[0] << 8) | (((UCHAR*)(mem))[1]))
#define READ_BE32(mem) ((((UCHAR*)(mem))[0] << 24) | (((UCHAR*)(mem))[1] << 16) | (((UCHAR*)(mem))[2] << 8) | (((UCHAR*)(mem))[3]))
#define P_MOVE(mem, a) ((mem) += (a))

#define READ_BE16_MOVE(mem) (READ_BE16((mem))); (P_MOVE((mem), 2))
#define READ_BE32_MOVE(mem) (READ_BE32((mem))); (P_MOVE((mem), 4))

uint8_t* PixelFont::loadFont(const char *fileName, int *fileSize) {
    if (strlen(fileName) > 0) {
        FILE *file = fopen(fileName, "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            int size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (fileSize) {
                *fileSize = size;
            }
            uint8_t* file_content = (uint8_t*)malloc(size + 1);
            int read_amount = fread(file_content, size, 1, file);
            file_content[size] = '\0';
            if (read_amount) {
                fclose(file);
                return file_content;
            }
            free(file_content);
            fclose(file);
            return NULL;
        }
    }
    return NULL;
}
