#include <stdio.h>

#include "view.h"

/* Print available commands and their arguments. */
void show_help(const char *program)
{
    printf("MP3 Tag Reader\n");
    puts("INFO: Help Menu for Tag Reader and Editor:\n");
    printf("View tags:  %s -v <file.mp3>\n", program);
    printf("Edit tags:  %s -e <modifier> \"new value\" <file.mp3>\n\n", program);
    puts("Modifiers:");
    puts("  -t  Title");
    puts("  -a  Artist");
    puts("  -A  Album");
    puts("  -y  Year");
    puts("  -m  Genre (Content)");
    puts("  -c  Comment");
    puts("");
    puts("Supported tags: ID3v1, ID3v2.2, ID3v2.3. ID3v2.4 is not supported.");
}

/* Print values collected by the parser. */
void show_tags(const Mp3Tags *tags)
{
    int i;

    printf("ID3 version: %s\n", tags->version);
    for (i = 0; i < TAG_COUNT; ++i) {
        printf("%s: %s\n", TAG_FRAME_IDS[i],
               tags->value[i] != NULL ? tags->value[i] : "(not set)");
    }
}
