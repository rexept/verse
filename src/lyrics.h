#ifndef LYRICS_H
#define LYRICS_H

#define MAX_LINE  1024
#define MAX_LINES 100000

void print_line_pair(char* lines[], char* artists[], char* titles[], int idx, int line_count, bool show_artist, bool show_title);
int  pick_random_line(char* lines[], int line_count);
int pick_random_lyric(const char* dir, char* out_line, char** out_artist, char** out_title);
int  load_lyrics(const char* dir, char* lines[], char* artists[], char* titles[], int max_lines);

#endif // LYRICS_H
