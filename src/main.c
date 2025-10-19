#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#define LYRICS_DIR "PENDING/.lyrics"
#define MAX_LINE 1024
#define MAX_LINES 100000  // adjust if you have tons of lyrics

int main() {
    srand(time(NULL));

    DIR *dir = opendir(LYRICS_DIR);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    char *lines[MAX_LINES];
    int line_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG) continue;
        if (!strstr(entry->d_name, ".txt")) continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", LYRICS_DIR, entry->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;

        char buffer[MAX_LINE];
        while (fgets(buffer, sizeof(buffer), f)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = 0;
            if (strlen(buffer) == 0) continue;

            if (line_count < MAX_LINES) {
                lines[line_count] = strdup(buffer);
                line_count++;
            }
        }
        fclose(f);
    }
    closedir(dir);

    if (line_count == 0) {
        printf("No lyrics found.\n");
        return 1;
    }

    int idx = rand() % line_count;
    printf("%s\n", lines[idx]);

    for (int i = 0; i < line_count; i++) free(lines[i]);

    return 0;
}
