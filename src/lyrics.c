#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

#include "lyrics.h"
#include "parser.h"

void print_line_pair(char* lines[], char* artists[], char* titles[], int idx, int line_count, bool show_artist, bool show_title) {
    (void)line_count;
    if (show_artist && show_title) {
        printf("%s - %s\n", artists[idx], titles[idx]);
    } else if (show_artist) {
        printf("%s\n", artists[idx]);
    } else if (show_title) {
        printf("%s\n", titles[idx]);
    }
    printf("%s\n", lines[idx]);
}

int pick_random_line(char* lines[], int line_count) {
    if (line_count <= 0)
        return 0;
    int idx;
    while (1) {
        idx = rand() % line_count;

        // skip bracketed lines
        if (lines[idx][0] == '[' || lines[idx][0] == '(')
            continue;

        if (idx + 1 < line_count) {
            if (lines[idx + 1][0] == '[' || lines[idx + 1][0] == '(')
                continue;
        }

        break;
    }
    return idx;
}

int pick_random_lyric(const char* dir, char* out_line, char** out_artist, char** out_title) {
    struct dirent** namelist;
    int             n = scandir(dir, &namelist, NULL, alphasort);
    if (n < 0) {
        perror("scandir");
        return 0;
    }

    // srand((unsigned)time(NULL));

    int total_seen = 0;
    out_line[0]    = '\0';
    *out_artist    = NULL;
    *out_title     = NULL;

    for (int i = 0; i < n; ++i) {
        struct dirent* entry = namelist[i];

        if (entry->d_type != DT_REG && entry->d_type != DT_UNKNOWN) {
            free(entry);
            continue;
        }
        if (!strstr(entry->d_name, ".txt")) {
            free(entry);
            continue;
        }

        char path[MAX_LINE];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        FILE* f = fopen(path, "r");
        if (!f) {
            free(entry);
            continue;
        }

        char* artist = NULL;
        char* title  = NULL;
        // Will always use filename splitter for metadata
        get_artist_title(entry->d_name, &artist, &title, true);

        char buffer[MAX_LINE];
        while (fgets(buffer, sizeof(buffer), f)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
                buffer[len - 1] = '\0';

            if (buffer[0] == '\0' || buffer[0] == '[' || buffer[0] == '(')
                continue;
            // In case lyric file has all lyrics on one line
            if (strlen(buffer) > 100)
                continue;

            total_seen++;

            if (rand() % total_seen == 0) {
                // This line becomes the current selection
                strncpy(out_line, buffer, MAX_LINE - 1);
                out_line[MAX_LINE - 1] = '\0';

                // Replace previous artist/title if they existed
                if (*out_artist)
                    free(*out_artist);
                if (*out_title)
                    free(*out_title);

                *out_artist = artist ? strdup(artist) : strdup("");
                *out_title  = title ? strdup(title) : strdup("");
            }
        }

        free(artist);
        free(title);
        fclose(f);
        free(entry);
    }

    free(namelist);

    return total_seen > 0 ? 1 : 0;
}

int load_lyrics(const char* dir, char* lines[], char* artists[], char* titles[], int max_lines) {
    struct dirent** namelist;
    int             n = scandir(dir, &namelist, NULL, alphasort);
    if (n < 0) {
        perror("scandir");
        return 0;
    }

    int  line_count = 0;
    char buffer[MAX_LINE];

    for (int i = 0; i < n; ++i) {
        struct dirent* entry = namelist[i];

        if (entry->d_type != DT_REG && entry->d_type != DT_UNKNOWN) {
            free(namelist[i]);
            continue;
        }
        if (!strstr(entry->d_name, ".txt")) {
            free(namelist[i]);
            continue;
        }

        char path[MAX_LINE];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        FILE* f = fopen(path, "r");
        if (!f) {
            free(namelist[i]);
            continue;
        }

        char* artist = NULL;
        char* title  = NULL;
        get_artist_title(entry->d_name, &artist, &title, false);

        while (fgets(buffer, sizeof(buffer), f)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
                buffer[len - 1] = '\0';
            if (buffer[0] == '\0')
                continue;

            if (line_count < max_lines) {
                lines[line_count]   = strdup(buffer);
                artists[line_count] = strdup(artist ? artist : "");
                titles[line_count]  = strdup(title ? title : "");
                ++line_count;
            } else {
                /* reached limit */
                break;
            }
        }
        free(artist);
        free(title);
        fclose(f);
        free(namelist[i]);
    }
    free(namelist);
    return line_count;
}
