#ifndef TAGREADER_H
#define TAGREADER_H

#ifdef WITH_TAGLIB
#include <taglib/tag_c.h>
#endif

int search_and_tag(const char* dir, const char* target_fname, char** artist, char** title);

#endif // TAGREADER_H
