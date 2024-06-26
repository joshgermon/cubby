#include <dirent.h>
#ifndef CUBCOPY_H

struct CopyOpts {
    char *include_only;
    char *exclude;
};

struct FileNode {
    int size;
    char *file_path;
    char *dest_path;
    char *file_name;
    int is_dir;
};

struct FileNodeArray {
    struct FileNode *nodes;
    int current;
    int length;
    size_t size;
};

struct FileNodeArray *init_file_node_array();

void free_file_node_array(struct FileNodeArray *array);

int discover_and_create_copy_queue(struct FileNodeArray *array,
char *src_dir,
char *dest_dir,
struct CopyOpts *opts);

int copy_contents(struct FileNodeArray *copy_queue,
void(*file_copied_callback)(struct FileNodeArray *copy_queue, int result));

int cc_copy(char *source_path, char *dest_path, struct CopyOpts opts);

#endif
