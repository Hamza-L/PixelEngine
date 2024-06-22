#include "PixelFont.h"
#include <cstdio>
#include <cstdlib>
#include <string.h>

// Read Big Endian Values (16bit, 32bit) or move pointer by (16bit, 32bit)
#define READ_BE16(mem) ((((UCHAR *)(mem))[0] << 8) | (((UCHAR *)(mem))[1]))
#define READ_BE32(mem) ((((UCHAR *)(mem))[0] << 24) | (((UCHAR *)(mem))[1] << 16) | (((UCHAR *)(mem))[2] << 8) | (((UCHAR *)(mem))[3]))
#define P_MOVE(mem, a) ((mem) += (a))

#define READ_BE16_MOVE(mem)                                                                                                                          \
    (READ_BE16((mem)));                                                                                                                              \
    (P_MOVE((mem), 2))
#define READ_BE32_MOVE(mem)                                                                                                                          \
    (READ_BE32((mem)));                                                                                                                              \
    (P_MOVE((mem), 4))

// Structs for the general table layout of the font data
typedef struct {
    UINT32 scaler_type;
    UINT16 numTables;
    UINT16 searchRange;
    UINT16 entrySelector;
    UINT16 rangeShift;
} offset_subtable;

typedef struct {
    union {
        CHAR tag_c[4];
        UINT32 tag;
    };
    UINT32 checkSum;
    UINT32 offset;
    UINT32 length;
} table_directory;

typedef struct {
    offset_subtable off_sub;
    table_directory *tbl_dir;
} font_directory;

// structs for the specific data concerning the subtables (ie glyphs)
typedef struct {
    UINT16 platformID;
    UINT16 platformSpecificID;
    UINT32 offset;
} cmap_encoding_subtable;

typedef struct {
    UINT16 version;
    UINT16 numberSubtables;
    cmap_encoding_subtable *subtables;
} cmap;

// start of each format
typedef struct {
    UINT16 format;
    UINT16 length;
    UINT16 language;
}; // not complete

typedef struct {
    UINT16 format;
    UINT16 length;
    UINT16 language;
    UINT16 segCountX2;
    UINT16 searchRange;
    UINT16 entrySelector;
    UINT16 rangeShift;
    UINT16 reservedPad;
    UINT16 *endCode;
    UINT16 *startCode;
    UINT16 *idDelta;
    UINT16 *idRangeOffset;
    UINT16 *glyphIdArray;
} format4;

void read_format4(UCHAR *mem, format4 **format) {
    UCHAR *m = mem;

    UINT16 length = READ_BE16(m + 2);
    format4 *f = NULL;

    f = (format4 *)calloc(1, length + sizeof(UINT16 *) * 5);
    f->format = READ_BE16_MOVE(m);
    f->length = READ_BE16_MOVE(m);
    f->language = READ_BE16_MOVE(m);
    f->segCountX2 = READ_BE16_MOVE(m);
    f->searchRange = READ_BE16_MOVE(m);
    f->entrySelector = READ_BE16_MOVE(m);
    f->rangeShift = READ_BE16_MOVE(m);

    f->endCode = (UINT16 *)((UINT8 *)f + sizeof(format4));
    f->startCode = f->endCode + f->segCountX2 / 2;
    f->idDelta = f->startCode + f->segCountX2 / 2;
    f->idRangeOffset = f->idDelta + f->segCountX2 / 2;
    f->glyphIdArray = f->idRangeOffset + f->segCountX2 / 2;

    UCHAR *start_code_start = m + f->segCountX2 + 2; // +2 for the reserved padding
    UCHAR *id_delta_start = m + f->segCountX2 * 2 + 2;
    UCHAR *id_range_start = m + f->segCountX2 * 3 + 2;

    for (int i = 0; i < f->segCountX2 / 2; ++i) {
        f->endCode[i] = READ_BE16(m + i * 2);
        f->startCode[i] = READ_BE16(start_code_start + i * 2);
        f->idDelta[i] = READ_BE16(id_delta_start + i * 2);
        f->idRangeOffset[i] = READ_BE16(id_range_start + i * 2);
    }

    P_MOVE(m, f->segCountX2 * 4 + 2);

    int remaining_bytes = f->length - (m - mem);
    // f->glyph_id_count = remaining_bytes/2;
    for (int i = 0; i < remaining_bytes / 2; ++i) {
        f->glyphIdArray[i] = READ_BE16_MOVE(m);
    }

    *format = f;
}

void print_format4(format4 *f4) {
    printf("Format: %d, Length: %d, Language: %d, Segment Count: %d\n", f4->format, f4->length, f4->language, f4->segCountX2 / 2);
    printf("Search Params: (searchRange: %d, entrySelector: %d, rangeShift: %d)\n", f4->searchRange, f4->entrySelector, f4->rangeShift);
    printf("Segment Ranges:\tstartCode\tendCode\tidDelta\tidRangeOffset\n");
    for (int i = 0; i < f4->segCountX2 / 2; ++i) {
        printf("--------------:\t% 9d\t% 7d\t% 7d\t% 12d\n", f4->startCode[i], f4->endCode[i], f4->idDelta[i], f4->idRangeOffset[i]);
    }
}

void read_cmap(UCHAR *mem, cmap *c) {
    UCHAR *m = mem;
    c->version = READ_BE16_MOVE(m);
    c->numberSubtables = READ_BE16_MOVE(m);

    c->subtables = (cmap_encoding_subtable *)calloc(1, sizeof(cmap_encoding_subtable) * c->numberSubtables);
    for (int i = 0; i < c->numberSubtables; ++i) {
        cmap_encoding_subtable *est = &c->subtables[i];
        est->platformID = READ_BE16_MOVE(m);
        est->platformSpecificID = READ_BE16_MOVE(m);
        est->offset = READ_BE32_MOVE(m);
    }
}

void read_offset_subtable(UCHAR **mem, offset_subtable *off_sub) {
    UCHAR *m = *mem;
    off_sub->scaler_type = READ_BE32_MOVE(m);
    off_sub->numTables = READ_BE16_MOVE(m);
    off_sub->searchRange = READ_BE16_MOVE(m);
    off_sub->entrySelector = READ_BE16_MOVE(m);
    off_sub->rangeShift = READ_BE16_MOVE(m);
    *mem = m;
}

void read_table_directory(UCHAR **mem, table_directory **tbl_dir, int tbl_size) {
    UCHAR *m = *mem;
    *tbl_dir = (table_directory *)calloc(1, sizeof(table_directory) * tbl_size);

    for (int i = 0; i < tbl_size; ++i) {
        table_directory *t = *tbl_dir + i;
        t->tag = READ_BE32_MOVE(m);
        t->checkSum = READ_BE32_MOVE(m);
        t->offset = READ_BE32_MOVE(m);
        t->length = READ_BE32_MOVE(m);
    }

    *mem = m;
}

static void read_font_directory(UCHAR **mem, font_directory *ftDirectory) {
    read_offset_subtable(mem, &ftDirectory->off_sub);
    read_table_directory(mem, &ftDirectory->tbl_dir, ftDirectory->off_sub.numTables);
}

void print_table_directory(table_directory *tbl_dir, int tbl_size) {
    printf("#)\ttag\tlen\toffset\n");
    for (int i = 0; i < tbl_size; ++i) {
        table_directory *t = tbl_dir + i;
        printf("%d)\t%c%c%c%c\t%d\t%d\n", i + 1, t->tag_c[3], t->tag_c[2], t->tag_c[1], t->tag_c[0], t->length, t->offset);
    }
}

void print_cmap(cmap *c) {
    printf("#)\tpId\tpsID\toffset\ttype\n");
    for (int i = 0; i < c->numberSubtables; ++i) {
        cmap_encoding_subtable *cet = c->subtables + i;
        printf("%d)\t%d\t%d\t%d\t", i + 1, cet->platformID, cet->platformSpecificID, cet->offset);
        switch (cet->platformID) {
        case 0:
            printf("Unicode");
            break;
        case 1:
            printf("Mac");
            break;
        case 2:
            printf("Not Supported");
            break;
        case 3:
            printf("Microsoft");
            break;
        }
        printf("\n");
    }
}

uint8_t *PixelFont::loadFont(const char *fileName, int *fileSize) {
    if (strlen(fileName) > 0) {
        FILE *file = fopen(fileName, "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            int size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (fileSize) {
                *fileSize = size;
            }
            UCHAR *file_content = (uint8_t *)malloc(size + 1);
            int read_amount = fread(file_content, size, 1, file);
            file_content[size] = '\0';

            UCHAR *file_ptr = file_content;
            font_directory fd = {};
            read_font_directory(&file_ptr, &fd);
            // print_table_directory(fd.tbl_dir, fd.tbl_dir->length);

            for (int i = 0; i < fd.off_sub.numTables; ++i) {
                if (fd.tbl_dir[i].tag == READ_BE32("cmap")) {
                    cmap c = {0};
                    file_ptr = file_content;
                    UCHAR* tbl_ptr = &file_ptr[fd.tbl_dir[i].offset];
                    read_cmap(tbl_ptr, &c); 
                    print_cmap(&c);
                    // read_format4();
                    // print_format4();
                }
            }

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
