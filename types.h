#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

/* The six fields exposed by this command-line reader. */
typedef enum {
    TAG_TITLE, TAG_ARTIST, TAG_ALBUM, TAG_YEAR, TAG_GENRE, TAG_COMMENT,
    TAG_COUNT
} TagField;

typedef struct {
    char *value[TAG_COUNT];
    char version[16];
    int has_v2;
    int has_v1;
} Mp3Tags;

extern const char *TAG_NAMES[TAG_COUNT];
extern const char *TAG_FRAME_IDS[TAG_COUNT];

int read_mp3_tags(const char *path, Mp3Tags *tags, char *error, size_t error_size);
void free_mp3_tags(Mp3Tags *tags);
int write_mp3_tag(const char *path, TagField field, const char *value,
                  char *error, size_t error_size);

#endif
