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

int load_lyrics(const char *dir, char *lines[], char *artists[], char *titles[],
                int max_lines);
int pick_random_line(char *lines[], int line_count);
void print_line_pair(char *lines[], char *artists[], char *titles[], int idx,
                     int line_count, bool show_artist, bool show_title);
char *init_config_dir(void);

static char *strdup_trim(const char *s);
static void split_artist_title(const char *filename, char **out_artist,
                               char **out_title);

int main(int argc, char *argv[]) {
  srand((unsigned)time(NULL));

  bool show_meta = false;
  bool show_artist = false;
  bool show_title = false;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--show-meta") == 0) {
      show_meta = true;
    } else if (strcmp(argv[i], "--show-artist") == 0) {
      show_artist = true;
    } else if (strcmp(argv[i], "--show-title") == 0) {
      show_title = true;
    }
  }

  /* --show-meta implies both artist and title */
  if (show_meta) {
    show_artist = show_title = true;
  }

  char *config = init_config_dir();
  if (!config) {
    fprintf(stderr, "Could not initialize config directory\n");
    return 1;
  }

  char *lines[MAX_LINES];
  static char *artists[MAX_LINES];
  static char *titles[MAX_LINES];

  int line_count = load_lyrics(LYRICS_DIR, lines, artists, titles, MAX_LINES);
  if (line_count == 0) {
    printf("No lyrics found.\n");
    free(config);
    return 1;
  }

  int idx = pick_random_line(lines, line_count);

  if (show_artist || show_title) {
    if (show_artist && artists[idx] && artists[idx][0] != '\0' && show_title &&
        titles[idx] && titles[idx][0] != '\0') {
      printf("%s - %s\n", artists[idx], titles[idx]);
    } else if (show_artist && artists[idx] && artists[idx][0] != '\0') {
      printf("%s\n", artists[idx]);
    } else if (show_title && titles[idx] && titles[idx][0] != '\0') {
      printf("%s\n", titles[idx]);
    }
  }

  // print_line_pair(lines, idx, line_count);
  printf("%s\n", lines[idx]);

  for (int i = 0; i < line_count; ++i) {
    free(lines[i]);
    free(artists[i]);
    free(titles[i]);
  }
  free(config);

  return 0;
}

/* ---------------------------------------------------------- */

int load_lyrics(const char *dir, char *lines[], char *artists[], char *titles[],
                int max_lines) {
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

    char *artist = NULL;
    char *title = NULL;
    split_artist_title(entry->d_name, &artist, &title);

    while (fgets(buffer, sizeof(buffer), f)) {
      size_t len = strlen(buffer);
      if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
      if (buffer[0] == '\0')
        continue;

      if (line_count < max_lines) {
        lines[line_count] = strdup(buffer);
        artists[line_count] = strdup(artist ? artist : "");
        titles[line_count] = strdup(title ? title : "");
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

/* ---------------------------------------------------------- */

int pick_random_line(char *lines[], int line_count) {
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

/* ---------------------------------------------------------- */

void print_line_pair(char *lines[], char *artists[], char *titles[], int idx,
                     int line_count, bool show_artist, bool show_title) {
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

/* ---------------------------------------------------------- */
/* strdup + trim whitespace */
static char *strdup_trim(const char *s) {
  if (!s)
    return strdup("");
  while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
    ++s;
  const char *end = s + strlen(s) - 1;
  while (end > s &&
         (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
    --end;
  size_t len = (end >= s) ? (size_t)(end - s) + 1 : 0;
  char *out = malloc(len + 1);
  if (!out)
    return NULL;
  memcpy(out, s, len);
  out[len] = '\0';
  return out;
}

/* ---------------------------------------------------------- */

static void split_artist_title(const char *filename, char **out_artist, char **out_title) {
    /* copy basename without extension */
    char base[PATH_MAX];
    strncpy(base, filename, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    /* strip trailing .txt (case-insensitive) */
    size_t L = strlen(base);
    if (L > 4) {
        char *dot = base + (int)(L - 4);
        if (strcasecmp(dot, ".txt") == 0) {
            *dot = '\0';
        }
    }

    /* try to split on " - " first occurrence */
    char *sep = strstr(base, " - ");
    if (!sep) {
        /* try just '-' */
        sep = strchr(base, '-');
    }

    if (sep) {
        /* artist = left side, title = right side */
        *sep = '\0';
        char *a = strdup_trim(base);
        char *t = strdup_trim(sep + 1);
        *out_artist = a ? a : strdup("");
        *out_title = t ? t : strdup("");
    } else {
        /* no separator: artist empty, title = base */
        *out_artist = strdup("");
        *out_title = strdup_trim(base);
        if (!*out_title) *out_title = strdup("");
    }
}
