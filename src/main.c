#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define LYRICS_DIR "PENDING/.lyrics"
#define CONFIG_DIR_NAME "verse"

#define MAX_LINE 1024
#define MAX_LINES 100000

int load_lyrics(const char *dir, char *lines[], int max_lines);
int pick_random_line(char *lines[], int line_count);
void print_line_pair(char *lines[], int idx, int line_count);
char *init_config_dir();

int main(int argc, char *argv[]) {
  srand(time(NULL));

  char *config = init_config_dir();
  if (!config) {
    fprintf(stderr, "Could not initialize config directory\n");
    return 1;
  }

  char *lines[MAX_LINES];
  int line_count = load_lyrics(LYRICS_DIR, lines, MAX_LINES);

  if (line_count == 0) {
    printf("No lyrics found.\n");
    return 1;
  }

  int idx = pick_random_line(lines, line_count);
  // print_line_pair(lines, idx, line_count);
  printf("%s\n", lines[idx]);

  for (int i = 0; i < line_count; i++)
    free(lines[i]);
  free(config);

  return 0;
}

/* ---------------------------------------------------------- */

int load_lyrics(const char *dir, char *lines[], int max_lines) {
  struct dirent **namelist;
  int n = scandir(dir, &namelist, NULL, alphasort);
  if (n < 0) {
    perror("scandir");
    return 0;
  }

  int line_count = 0;
  char buffer[MAX_LINE];

  for (int i = 0; i < n; ++i) {
    struct dirent *entry = namelist[i];

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

    FILE *f = fopen(path, "r");
    if (!f) {
      free(namelist[i]);
      continue;
    }

    while (fgets(buffer, sizeof(buffer), f)) {
      size_t len = strlen(buffer);
      if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
      if (strlen(buffer) == 0)
        continue;
      if (line_count < max_lines)
        lines[line_count++] = strdup(buffer);
    }
    fclose(f);
    free(namelist[i]);
  }
  free(namelist);
  return line_count;
}

/* ---------------------------------------------------------- */

int pick_random_line(char *lines[], int line_count) {
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

/* ---------------------------------------------------------- */

void print_line_pair(char *lines[], int idx, int line_count) {
  printf("%s\n", lines[idx]);

  if (idx + 1 < line_count) {
    if (lines[idx + 1][0] != '[' && lines[idx + 1][0] != '(') {
    }
  }
}

/* ---------------------------------------------------------- */

char *init_config_dir() {
  const char *xdg = getenv("XDG_CONFIG_HOME");
  char *config_dir = NULL;

  if (xdg && xdg[0] != '\0') {
    config_dir = malloc(strlen(xdg) + 1 + strlen(CONFIG_DIR_NAME) + 1);
    if (!config_dir)
      return NULL;
    sprintf(config_dir, "%s/%s", xdg, CONFIG_DIR_NAME);
  } else {
    const char *home = getenv("HOME");
    if (!home)
      return NULL;
    config_dir = malloc(strlen(home) + 1 + strlen(".config/") +
                        strlen(CONFIG_DIR_NAME) + 1);
    if (!config_dir)
      return NULL;
    sprintf(config_dir, "%s/.config/%s", home, CONFIG_DIR_NAME);
  }

  // create the directory if it doesn't exist
  struct stat st;
  if (stat(config_dir, &st) != 0) {
    if (mkdir(config_dir, 0700) != 0) {
      fprintf(stderr, "Failed to create config dir %s: %s\n", config_dir,
              strerror(errno));
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
