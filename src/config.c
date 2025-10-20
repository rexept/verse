#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"

char* init_config_dir() {
    const char* xdg        = getenv("XDG_CONFIG_HOME");
    char*       config_dir = NULL;

    if (xdg && xdg[0] != '\0') {
        config_dir = malloc(strlen(xdg) + 1 + strlen(CONFIG_DIR_NAME) + 1);
        if (!config_dir)
            return NULL;
        sprintf(config_dir, "%s/%s", xdg, CONFIG_DIR_NAME);
    } else {
        const char* home = getenv("HOME");
        if (!home)
            return NULL;
        config_dir = malloc(strlen(home) + 1 + strlen(".config/") + strlen(CONFIG_DIR_NAME) + 1);
        if (!config_dir)
            return NULL;
        sprintf(config_dir, "%s/.config/%s", home, CONFIG_DIR_NAME);
    }

    // create the directory if it doesn't exist
    struct stat st;
    if (stat(config_dir, &st) != 0) {
        if (mkdir(config_dir, 0700) != 0) {
            fprintf(stderr, "Failed to create config dir %s: %s\n", config_dir, strerror(errno));
            free(config_dir);
            return NULL;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "%s exists but is not a directory\n", config_dir);
        free(config_dir);
        return NULL;
    }

    return config_dir;
}
