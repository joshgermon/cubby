#include "../include/cubby.h"
#include "../include/cubcopy.h"
#include "../include/utils.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define ISO_TIMESTAMP_LENGTH 12

static void create_timestamp_backup_dir(const char *backup_path, char *out_path, size_t out_path_len);

void copy_callback(struct FileNodeArray *copy_queue, int copy_result) {
  if(copy_result == 0) {
    printf("Copied file (%d of %d) ... \n", copy_queue->current - 1, copy_queue->length);
  } else {
    printf("Failed to copy file (%d of %d), skipping...\n", copy_queue->current - 1, copy_queue->length);
  }
}

int backup_dir(cubby_opts_t *opts, char *source_dir)
{
    size_t backup_path_len = strlen(opts->backup_path) + ISO_TIMESTAMP_LENGTH + 2;
    char backup_path[backup_path_len];
    create_timestamp_backup_dir(opts->backup_path, backup_path, backup_path_len);

    printf("Backing up with cubcopy from '%s' to '%s'\n", source_dir, backup_path);

    struct FileNodeArray *copy_queue = init_file_node_array();
    if (discover_and_create_copy_queue(copy_queue, source_dir, backup_path, NULL) == -1) {
      free_file_node_array(copy_queue);
      return 1;
    }

    printf("Starting copy of %d files...\n", copy_queue->length);

    copy_contents(copy_queue, copy_callback);

    printf("Completed cubcopy backup without error.\n");

    free_file_node_array(copy_queue);
    return 0;
}

static void create_timestamp_backup_dir(const char *backup_path, char *out_path, size_t out_path_len)
{
    time_t t           = time(NULL);
    struct tm *tm_info = localtime(&t);

    /** Get YYYY-MM-DD timestamp for directory name */
    char date_str[ISO_TIMESTAMP_LENGTH];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);

    /** Create the full backup path */
    snprintf(out_path, out_path_len, "%s/%s", backup_path, date_str);

    /** Ensure dated directory does not already exist at backup path
        If it does, append a number until it's unique */
    struct stat st = { 0 };
    int counter    = 0;
    while (stat(out_path, &st) != -1) {
        if (counter >= 10)
            die("Retried creating directory too many times");
        printf("Directory exists, trying with counter appended...\n");
        snprintf(out_path, out_path_len, "%s/%s_%d", backup_path, date_str, counter);
        counter++;
    }

    if (mkdir(out_path, 0755) == -1)
        die("Failed to create directory");

    printf("Directory created with timestamp at '%s'\n", out_path);
}

const char *mount_device_to_fs(const char *source)
{
    /* Generate unique id for mount path */
    char uid[UID_LENGTH];
    generate_unique_id_string(uid, sizeof(uid));

    static char mount_path[sizeof(uid) + 5];
    snprintf(mount_path, sizeof(mount_path), "/mnt/%s", uid);

    printf("Attempting to mount to path: %s\n", mount_path);
    if (mkdir(mount_path, 0755) == -1) {
        die("Failed to create mount directory");
    }

    // TODO: Mount different file types instead of hardcode exfat
    if (mount(source, mount_path, "exfat", 0, "") == 0) {
        printf("Device successfully mounted at %s\n", mount_path);
    } else {
        perror("Mount failed");
        return NULL;
    }

    return mount_path;
}

void unmount_device(const char *mount_path)
{
    if (umount(mount_path) == 0) {
        printf("Unmounted device from '%s' successfully.\n", mount_path);
        if (rmdir(mount_path) == 0) {
            printf("Removed temp mount dir '%s' successfully.\n", mount_path);
        } else {
            perror("Attempt to cleanup mount directory failed");
        }
    } else {
        perror("Attempt to unmount device failed");
    }
}
