#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lyrics.h"
#include "config.h"
#include "parser.h"

static void print_help(void);

/* --------------------------------------- */

int main(int argc, char* argv[]) {
    srand((unsigned)time(NULL));

    bool show_meta   = false;
    bool show_artist = false;
    bool show_title  = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--show-meta") == 0) {
            show_meta = true;
        } else if (strcmp(argv[i], "--show-artist") == 0) {
            show_artist = true;
        } else if (strcmp(argv[i], "--show-title") == 0) {
            show_title = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("verse-" VERSION);
            return 0;
        }
    }

    /* --show-meta implies both artist and title */
    if (show_meta) {
        show_artist = show_title = true;
    }

    char* config = init_config_dir();
    if (!config) {
        fprintf(stderr, "Could not initialize config directory\n");
        return 1;
    }

    char         lines[MAX_LINES];
    static char* artist = NULL;
    static char* title  = NULL;
    //

    if (!pick_random_lyric(LYRICS_DIR, lines, &artist, &title)) {
        printf("No lyrics found");
        return 1;
    }

    if (show_artist || show_title) {
        if (show_artist && show_title && artist && title) {
            printf("%s - %s\n", artist, title);
        } else if (show_artist && artist) {
            printf("%s\n", artist);
        } else if (show_title && title) {
            printf("%s\n", title);
        }
    }

    // print_line_pair(lines, idx, line_count);
    printf("%s\n", lines);

    free(artist);
    free(title);
    free(config);

    return 0;
}

static void print_help(void) {
    printf("Usage: verse [FLAGS]\n");
    printf("\nDisplay a random line from your lyrics database.\n\n");
    printf("Options:\n");
    printf("  --show-artist       Display the artist along with the quote\n");
    printf("  --show-title        Display the song title along with the quote\n");
    printf("  --show-meta         Display both the song title and artist with the quote\n");
    printf("  -h, --help          Show this help message and exit\n");
    printf("  -v, --version       Display the current version of verse\n");
}
