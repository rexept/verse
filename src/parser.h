#ifndef PARSER_H
#define PARSER_H

#define MUSIC_DIR "PENDING/music/tracks"

void         get_artist_title(const char* filename, char** artist, char** title, bool force_split_filename);
int          ends_with(const char* str, const char* suffix);
char*        remove_extension(const char* filename);
void         split_artist_title(const char* filename, char** out_artist, char** out_title);
static char* strdup_trim(const char* s);

#endif // PARSER_H
