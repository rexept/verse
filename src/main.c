#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LYRICS_DIR "PENDING/.lyrics"

#define MAX_LINE 1024
#define MAX_LINES 100000

int load_lyrics(const char *dir, char *lines[], int max_lines);
int pick_random_line(char *lines[], int line_count);
void print_line_pair(char *lines[], int idx, int line_count);

int main() {
  srand(time(NULL));

  char *lines[MAX_LINES];
  int line_count = load_lyrics(LYRICS_DIR, lines, MAX_LINES);

  if (line_count == 0) {
    printf("No lyrics found.\n");
    return 1;
  }

  int idx = pick_random_line(lines, line_count);
  print_line_pair(lines, idx, line_count);

  for (int i = 0; i < line_count; i++)
    free(lines[i]);

  return 0;
}

/* ---------------------------------------------------------- */

int load_lyrics(const char *dir, char *lines[], int max_lines) {
  DIR *dp = opendir(dir);
  if (!dp) {
    perror("opendir");
    return 0;
  }

  struct dirent *entry;
  int line_count = 0;
  char buffer[MAX_LINE];

  while ((entry = readdir(dp)) != NULL) {
    if (entry->d_type != DT_REG)
      continue;
    if (!strstr(entry->d_name, ".txt"))
      continue;

    char path[MAX_LINE];
    snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

    FILE *f = fopen(path, "r");
    if (!f)
      continue;

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
  }

  closedir(dp);
  return line_count;
}

int pick_random_line(char *lines[], int line_count) {
  int idx;
  while (true) {
    idx = rand() % line_count;

    if (lines[idx][0] == '[' || lines[idx][0] == '(')
      continue;

    if (idx + 1 < line_count && lines[idx + 1][0] == '[' ||
        lines[idx][0] == '(')
      continue;

    break;
  }
  return idx;
}

void print_line_pair(char *lines[], int idx, int line_count) {
  printf("%s\n", lines[idx]);
  if (idx + 1 < line_count && lines[idx + 1][0] != '[' || lines[idx][0] != '(')
    printf("%s\n", lines[idx + 1]);
}
