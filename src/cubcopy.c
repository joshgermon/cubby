#include "../include/cubcopy.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_PATH_LENGTH 1024
#define DEFAULT_CHUNK_SIZE 8096
#define INITIAL_CAPACITY 1024

int file_matches_filter(const char *filename, struct CopyOpts *opts);

struct FileNodeArray *init_file_node_array() {
  struct FileNodeArray *array = malloc(sizeof(struct FileNodeArray));
  array->length = 0;
  array->current = 0;
  array->size = INITIAL_CAPACITY;
  array->nodes = malloc(array->size * sizeof(struct FileNode));
  return array;
}

// Function to add a node to the FileNodeArray
void append_file_node(struct FileNodeArray *array, struct FileNode *node) {
  if (array->length == array->size) {
    array->size *= 2;
    array->nodes = realloc(array->nodes, array->size * sizeof(struct FileNode));
  }
  array->nodes[array->length++] = *node;
}

// Function to get the next node from the FileNodeArray
struct FileNode *next_file_node(struct FileNodeArray *array) {
  if (array->current < array->length) {
    return &array->nodes[array->current++];
  }
  return NULL;
}

// Function to free the FileNodeArray
void free_file_node_array(struct FileNodeArray *array) {
  // Free each node's file path and file name
  for (int i = 0; i < array->length; i++) {
    free(array->nodes[i].file_path);
    free(array->nodes[i].file_name);
  }
  free(array->nodes);
  free(array);
}

int discover_and_create_copy_queue(struct FileNodeArray *array, char *src_dir,
                                   char *dest_dir, struct CopyOpts *opts) {
  // Try and open the directory
  DIR *dir = opendir(src_dir);
  if (!dir) {
    perror("opendir");
    return -1;
  }

  // Read the entries of the current directory
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    // Append the directory/file name to the current path to get entry's full path
    char *source_path = malloc(MAX_PATH_LENGTH);
    snprintf(source_path, MAX_PATH_LENGTH, "%s/%s", src_dir, entry->d_name);

    char *dest_path = malloc(MAX_PATH_LENGTH);
    snprintf(dest_path, MAX_PATH_LENGTH, "%s/%s", dest_dir, entry->d_name);

    // Try and read the stats of current entry
    struct stat path_stat;
    if (stat(source_path, &path_stat) == -1) {
      perror("stat");
      free(source_path);
      // WARN: Failed to get stats for file, skipping
      continue;
    }

    int is_dir = S_ISDIR(path_stat.st_mode);

    // Check if the file matches filter (also filter out '.' entries)
    if (!is_dir && !file_matches_filter(entry->d_name, opts)) {
      // TODO: Log that file was filtered out
      free(source_path);
      continue;
    }

    ssize_t file_name_len = strlen(entry->d_name) + 1;
    char *file_name = malloc(file_name_len);
    strncpy(file_name, entry->d_name, file_name_len);

    // Create file node and add to copy queue
    struct FileNode fi = {.file_name = file_name,
                      .size = path_stat.st_size,
                      .file_path = source_path,
                      .dest_path = dest_path,
                      .is_dir = is_dir};

    append_file_node(array, &fi);

    if(is_dir) {
      // Recursively call the function to discover files in the directory
      discover_and_create_copy_queue(array, source_path, dest_path, opts);
    }
  }
  closedir(dir);
  return 0;
}

const char *get_file_ext(const char *filename) {
  const char *dot = strrchr(filename, '.'); // Find the last occurrence of '.'
  if (!dot || dot == filename)
    return NULL;  // If there's no dot or dot is at the beginning, return NULL
  return dot + 1; // Return the extension (skip the dot)
}

// TODO: Function a bit of a mess
int file_matches_filter(const char *filename, struct CopyOpts *opts) {
  // Can be expanded, right now just checking file extension
  if (opts->include_only != NULL) {
    const char *file_ext = get_file_ext(filename);
    if (file_ext == NULL)
      return 0;
    if (strcmp(file_ext, opts->include_only) == 0)
      return 1;
    return 0;
  }
  return 1;
}

int copy_file(const char *src_path, const char *dest_path) {
  FILE *source_file = fopen(src_path, "rb");
  FILE *destination_file = fopen(dest_path, "w+b");

  if (source_file == NULL || destination_file == NULL) {
    perror("Error opening file");
    return -1;
  }

  char buffer[DEFAULT_CHUNK_SIZE];
  size_t bytes_read;
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
    fwrite(buffer, 1, bytes_read, destination_file);
  }
  fclose(source_file);
  fclose(destination_file);

  return 0;
}

int copy_contents(struct FileNodeArray *copy_queue, void (*file_copied_callback)(struct FileNodeArray *copy_queue, int result)) {
  struct FileNode *current_node = next_file_node(copy_queue);
  while (current_node != NULL) {
    if (current_node->is_dir) {
      if(mkdir(current_node->dest_path, 0755) != 0) {
        perror("mkdir");
        return -1;
      }
    } else {
      // Copy file to target
      int file_copy_result = copy_file(current_node->file_path, current_node->dest_path);
      if(file_copied_callback != NULL) (file_copied_callback(copy_queue, file_copy_result));
    }
    current_node = next_file_node(copy_queue);
  }
  return 0;
}

int cc_copy(char *source_path, char *dest_path, struct CopyOpts opts) {
  struct FileNodeArray *copy_queue = init_file_node_array();
  if (discover_and_create_copy_queue(copy_queue, source_path, dest_path, &opts) == -1) {
    free_file_node_array(copy_queue);
    return 1;
  }

  return copy_contents(copy_queue, NULL);
}

