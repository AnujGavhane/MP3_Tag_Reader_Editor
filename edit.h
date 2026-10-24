#ifndef EDIT_H
#define EDIT_H
#include "types.h"
int write_mp3_tag(const char *path, TagField field, const char *value,
                 char *error, size_t error_size);
#endif
