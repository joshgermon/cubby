#include <stdio.h>
#include <stdlib.h>


void die(char *err)
{
    fprintf(stderr, "Error: %s\n", err);
    exit(EXIT_FAILURE);
}

void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL) {
        die("Failed to malloc");
    }
    return p;
}

void *xrealloc(void *p, size_t size)
{
    void *np = realloc(p, size);
    if (np == NULL) {
        free(p);
        die("Failed to realloc");
    }
    return np;
}
