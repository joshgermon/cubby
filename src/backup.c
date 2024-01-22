#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <dirent.h>

// TODO: Remove, just for testing purposes
int print_dir_contents() {
  // Directory path
  const char *path = "/mnt";

  // Open the directory
  DIR *directory = opendir(path);

  // Check if the directory is opened successfully
  if (directory != NULL) {
    // Read directory entries
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
      // Print file names
      printf("%s\n", entry->d_name);
    }

    // Close the directory
    closedir(directory);
  } else {
    // Handle error if directory cannot be opened
    perror("Error opening directory");
    return 1;
  }

  return 0;
}

int mount_device_to_fs(const char *source) {
  // TODO: Make this random uid
  const char *mount_path = "/mnt";

  if (mount("/dev/sdc1", mount_path, "exfat", 0, "") == 0) {
    printf("Mount successful!\n");
  } else {
    perror("Mount failed");
  }

  print_dir_contents();

  if (umount(mount_path) == 0) {
    printf("Unmount successful!\n");
  } else {
    perror("Unmount failed");
    exit(EXIT_FAILURE);
  }

  return 0;
}
