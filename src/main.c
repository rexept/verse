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
#define PROFANITY_FLAGS_CACHE "profanity_flags_cache.txt"

#define MAX_LINE 1024
#define MAX_LINES 100000

int load_lyrics(const char *dir, char *lines[], int max_lines);
int pick_random_line(char *lines[], int line_count, bool allow_profanities);
void print_line_pair(char *lines[], int idx, int line_count,
                     bool allow_profanities);
void load_profanity_flags(const char *filename, int profane_flags[],
                          int line_count);
void generate_profanity_flags(char *lines[], int line_count,
                              int profane_flags[], char *cache_path);
int copy_file(const char *src, const char *dst);
char *init_config_dir();

// Array to store cached profanity flags (0 = clean, 1 = profane)
int profane_flags[MAX_LINES];

int main(int argc, char *argv[]) {
  srand(time(NULL));

  char *config = init_config_dir();
  if (!config) {
    fprintf(stderr, "Could not initialize config directory\n");
    return 1;
  }

  bool allow_profanities = false;
  bool need_generate = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--allow-profanities") == 0) {
      allow_profanities = true;
    }
    if (strcmp(argv[i], "--generate-profanity-cache") == 0) {
      need_generate = true;
    }
  }

  char *lines[MAX_LINES];
  int line_count = load_lyrics(LYRICS_DIR, lines, MAX_LINES);

  if (line_count == 0) {
    printf("No lyrics found.\n");
    return 1;
  }

  char cache_file[PATH_MAX];
  snprintf(cache_file, sizeof(cache_file), "%s/%s", config,
           PROFANITY_FLAGS_CACHE);

  /* if cache missing or size mismatch -> regenerate */
  struct stat st;
  if (stat(cache_file, &st) != 0) {
    need_generate = true;
  } else {
    /* quick check: count lines in cache_file */
    FILE *pf = fopen(cache_file, "r");
    int cached_lines = 0;
    char tmp[16];
    while (pf && fgets(tmp, sizeof(tmp), pf))
      cached_lines++;
    if (pf)
      fclose(pf);
    if (cached_lines != line_count)
      need_generate = true;
  }

  if (need_generate && !allow_profanities) {
    printf("Profanity cache needs generating. Generating now... (this may take "
           "a while depending on how large your lyrics database is)\n");
    generate_profanity_flags(lines, line_count, profane_flags, cache_file);
    FILE *out = fopen(cache_file, "w");
    if (out) {
      for (int i = 0; i < line_count; ++i)
        fprintf(out, "%d\n", profane_flags[i]);
      fclose(out);
    }
  } else {
    load_profanity_flags(cache_file, profane_flags, line_count);
  }
  printf("%c", profane_flags[0]);

  int idx = pick_random_line(lines, line_count, allow_profanities);
  print_line_pair(lines, idx, line_count, allow_profanities);

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

void load_profanity_flags(const char *cache_path, int profane_flags[],
                          int line_count) {
  FILE *pf = fopen(cache_path, "r");
  if (pf) {
    for (int i = 0; i < line_count; i++) {
      char buf[16];
      if (fgets(buf, sizeof(buf), pf))
        profane_flags[i] = atoi(buf);
      else
        profane_flags[i] = 0;
    }
    fclose(pf);
  } else {
    // cache not found -> init to clean
    for (int i = 0; i < line_count; i++)
      profane_flags[i] = 0;
  }
}

/* ---------------------------------------------------------- */

int pick_random_line(char *lines[], int line_count, bool allow_profanities) {
  int idx;
  while (1) {
    idx = rand() % line_count;

    // skip bracketed lines
    if (lines[idx][0] == '[' || lines[idx][0] == '(')
      continue;

    // skip profanities unless allowed
    if (!allow_profanities && profane_flags[idx])
      continue;

    // skip if next line is bracketed or profane
    if (idx + 1 < line_count) {
      if (lines[idx + 1][0] == '[' || lines[idx + 1][0] == '(')
        continue;
      if (!allow_profanities && profane_flags[idx + 1])
        continue;
    }

    break;
  }
  return idx;
}

/* ---------------------------------------------------------- */

void print_line_pair(char *lines[], int idx, int line_count,
                     bool allow_profanities) {
  printf("%s\n", lines[idx]);

  if (idx + 1 < line_count) {
    if (lines[idx + 1][0] != '[' && lines[idx + 1][0] != '(') {
      if (allow_profanities || !profane_flags[idx + 1])
        printf("%s\n", lines[idx + 1]);
    }
  }
}

/* ---------------------------------------------------------- */

void generate_profanity_flags(char *lines[], int line_count,
                              int profane_flags[], char *cache_path) {
  char temp_input[] = "/tmp/lyrics_input.txt";

  // write all lines to temp input file
  FILE *f = fopen(temp_input, "w");
  if (!f) {
    perror("fopen temp_input");
    for (int i = 0; i < line_count; i++)
      profane_flags[i] = 0;
    return;
  }
  for (int i = 0; i < line_count; i++)
    fprintf(f, "%s\n", lines[i]);
  fclose(f);

  // call separate Python script to generate profanity flags
  char cmd[4096];
  snprintf(cmd, sizeof(cmd), "python3 check_profanity.py %s %s", temp_input,
           cache_path);

  int ret = system(cmd);
  if (ret != 0) {
    fprintf(stderr,
            "Python profanity check failed, marking all lines clean.\n");
    for (int i = 0; i < line_count; i++)
      profane_flags[i] = 0;
    return;
  }

  // read the flags back
  FILE *pf = fopen(cache_path, "r");
  if (pf) {
    for (int i = 0; i < line_count; i++) {
      char buf[16];
      if (fgets(buf, sizeof(buf), pf))
        profane_flags[i] = atoi(buf);
      else
        profane_flags[i] = 0;
    }
    fclose(pf);
  } else {
    for (int i = 0; i < line_count; i++)
      profane_flags[i] = 0;
  }

  remove(temp_input);
}

int copy_file(const char *src, const char *dst) {
  FILE *in = fopen(src, "rb");
  if (!in)
    return 1;
  FILE *out = fopen(dst, "wb");
  if (!out) {
    fclose(in);
    return 1;
  }

  char buf[4096];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
    if (fwrite(buf, 1, n, out) != n) {
      fclose(in);
      fclose(out);
      return 1;
    }
  }

  fclose(in);
  fclose(out);
  return 0;
}

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
