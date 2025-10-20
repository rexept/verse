#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "tagreader.h"

void get_artist_title(const char* filename, char** artist, char** title, bool force_split_filename) {
    if (force_split_filename) {
        split_artist_title(filename, artist, title);
        return;
    }

    char* target_fname = remove_extension(filename);
    if (!target_fname) {
        split_artist_title(filename, artist, title);
        return;
    }

    if (!search_and_tag(MUSIC_DIR, target_fname, artist, title)) {
        split_artist_title(filename, artist, title);
    }
    free(target_fname);
}

char* remove_extension(const char* filename) {
    char* dot = strrchr(filename, '.');
    if (!dot)
        return strdup(filename);
    size_t len    = dot - filename;
    char*  result = malloc(len + 1);
    if (!result)
        return NULL;
    strncpy(result, filename, len);
    result[len] = '\0';
    return result;
}

int ends_with(const char* str, const char* suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr    = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void split_artist_title(const char* filename, char** out_artist, char** out_title) {
    /* copy basename without extension */
    char base[PATH_MAX];
    strncpy(base, filename, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    /* strip trailing .txt (case-insensitive) */
    size_t L = strlen(base);
    if (L > 4) {
        char* dot = base + (int)(L - 4);
        if (strcasecmp(dot, ".txt") == 0) {
            *dot = '\0';
        }
    }

    /* try to split on " - " first occurrence */
    char* sep = strstr(base, " - ");
    if (!sep) {
        /* try just '-' */
        sep = strchr(base, '-');
    }

    if (sep) {
        /* artist = left side, title = right side */
        *sep        = '\0';
        char* a     = strdup_trim(base);
        char* t     = strdup_trim(sep + 1);
        *out_artist = a ? a : strdup("");
        *out_title  = t ? t : strdup("");
    } else {
        /* no separator: artist empty, title = base */
        *out_artist = strdup("");
        *out_title  = strdup_trim(base);
        if (!*out_title)
            *out_title = strdup("");
    }
}

static char* strdup_trim(const char* s) {
    if (!s)
        return strdup("");
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
        ++s;
    const char* end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        --end;
    size_t len = (end >= s) ? (size_t)(end - s) + 1 : 0;
    char*  out = malloc(len + 1);
    if (!out)
        return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}
