#include <stdio.h>
#include <stdlib.h>
#include <time.h>


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

void generate_unique_id_string(char *buffer, size_t buffer_size) {
    // Seed the random number generator with the current time
    srand(time(NULL));
    // Generate a unique ID based on the current time and a random number
    unsigned long unique_id = time(NULL) + rand();
    // Use snprintf to safely format the unique ID into a string
    snprintf(buffer, buffer_size, "%lu", unique_id);
}

