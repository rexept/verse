#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#include "lyrics.h"
#include "config.h"

static void print_help(void);

/* --------------------------------------- */

typedef struct {
  int f_show_artist;
  int f_show_title;
  int f_show_meta;
} meta;

int main(int argc, char* argv[]) {
    srand((unsigned)time(NULL));

    static meta m;
    int c;

    for (;;) {
        int option_index = 0;
        static struct option long_options[] = {
            {"show-meta", no_argument, &m.f_show_meta, true},
            {"show-artist", no_argument, &m.f_show_artist, true},
            {"show-title", no_argument, &m.f_show_title, true},
            {"version", no_argument, NULL, 'v'},
            {"help", no_argument, NULL, 'h'},
            {NULL, 0, NULL, 0}
        };

        c = getopt_long(argc, argv, "vh", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                break;
            case 'v':
                printf("verse-" VERSION);
                return 0;
            case 'h':
                print_help();
                return 0;
            default:
                fprintf(stderr, "Usage: %s [FLAGS]\nSee help for more info", argv[0]);
                return EXIT_SUCCESS;
        }
    }

    /* --show-meta implies both artist and title */
    if (m.f_show_meta) {
        m.f_show_artist = m.f_show_title = true;
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

    if (m.f_show_artist || m.f_show_title) {
        if (m.f_show_artist && m.f_show_title && artist && title) {
            printf("%s - %s\n", artist, title);
        } else if (m.f_show_artist && artist) {
            printf("%s\n", artist);
        } else if (m.f_show_title && title) {
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
