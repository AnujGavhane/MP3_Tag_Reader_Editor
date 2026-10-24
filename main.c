#include <stdio.h>
#include <string.h>

#include "types.h"
#include "view.h"
#include "edit.h"

/* Convert the modifier from the example command format into a tag field. */
static int get_field(const char *modifier)
{
    if (strcmp(modifier, "-t") == 0) return TAG_TITLE;
    if (strcmp(modifier, "-a") == 0) return TAG_ARTIST;
    if (strcmp(modifier, "-A") == 0) return TAG_ALBUM;
    if (strcmp(modifier, "-y") == 0) return TAG_YEAR;
    if (strcmp(modifier, "-m") == 0) return TAG_GENRE;
    if (strcmp(modifier, "-c") == 0) return TAG_COMMENT;
    return -1;
}

int main(int argc, char **argv)
{
    int field;
    char error[256];
    Mp3Tags tags;

    if (argc < 2 || strcmp(argv[1], "-h") == 0 ||
        strcmp(argv[1], "--help") == 0) {
        show_help(argv[0]);
        return argc < 2 ? 1 : 0;
    }

    /* View mode: -v file.mp3 */
    if (strcmp(argv[1], "-v") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s -v <file.mp3>\n", argv[0]);
            return 1;
        }
        if (!read_mp3_tags(argv[2], &tags, error, sizeof(error))) {
            fprintf(stderr, "Error: %s\n", error);
            return 1;
        }
        show_tags(&tags);
        free_mp3_tags(&tags);
        return 0;
    }

    /* Edit mode: -e modifier "new value" file.mp3 */
    if (strcmp(argv[1], "-e") == 0) {
        if (argc < 5) {
            fprintf(stderr,
                    "Usage: %s -e <modifier> \"new value\" <file.mp3>\n",
                    argv[0]);
            return 1;
        }
        field = get_field(argv[2]);
        if (field < 0) {
            fprintf(stderr, "Unknown modifier: %s\n", argv[2]);
            return 1;
        }
        if (!write_mp3_tag(argv[4], (TagField)field, argv[3],
                           error, sizeof(error))) {
            fprintf(stderr, "Error: %s\n", error);
            return 1;
        }
        puts("INFO: Tag edited successfully");
        return 0;
    }

    fprintf(stderr, "Invalid arguments (use --help for help)\n");
    return 1;
}
