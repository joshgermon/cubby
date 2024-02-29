#include <stdlib.h>

#define UID_LENGTH 21

void generate_unique_id_string(char *buffer, size_t buffer_size);

void die (char *err);

void *xmalloc (size_t size);

void *xrealloc (void *p, size_t size);
