#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "tagreader.h"
#include "parser.h"

int search_and_tag(const char* dir, const char* target_fname, char** artist, char** title) {
    DIR* dp = opendir(dir);
    if (!dp)
        return 0;

    struct dirent* entry;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        struct stat st;
        if (stat(path, &st) == -1) {
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (search_and_tag(path, target_fname, artist, title)) {
                closedir(dp);
                return 1;
            }
        } else if (S_ISREG(st.st_mode)) {
            if (ends_with(entry->d_name, ".mp3")) {
                char* name_no_ext = remove_extension(entry->d_name);
                if (name_no_ext) {
                    if (strcmp(name_no_ext, target_fname) == 0) {
#ifdef WITH_TAGLIB
                        TagLib_File* file = taglib_file_new(path);
                        if (file) {
                            const TagLib_Tag* tag = taglib_file_tag(file);
                            if (tag) {
                                const char* a = taglib_tag_artist(tag);
                                const char* t = taglib_tag_title(tag);
                                if (a && t) {
                                    *artist = strdup(a);
                                    *title  = strdup(t);
                                    taglib_file_free(file);
                                    free(name_no_ext);
                                    closedir(dp);
                                    return 1;
                                }
                            }
                            taglib_file_free(file);
                        }
#endif
                    }
                    free(name_no_ext);
                }
            }
        }
    }
    closedir(dp);
    return 0;
}
