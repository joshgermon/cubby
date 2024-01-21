#include "systemd/sd-device.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>

#define SUBSYSTEM "block"
#define DEVICE_TYPE "partition"
// #define DEVICE_TYPE "disk"

typedef struct {
  char *id_name;
  char *uuid;
  char *size;
  char *block_size_part_table_type;
  char *syspath;
} device_attrs;

// TODO: Remove
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

static device_attrs get_sdcard_attributes(sd_device *dev) {
  device_attrs dev_attrs = {"noname", "noserial", "nosize",
                            "noblocksizenorparttabletype", "no-path"};
  int res;
  const char *syspath;
  res = sd_device_get_syspath(dev, &syspath);
  if (res >= 0)
    dev_attrs.syspath = (char *)syspath;

  const char *name;
  res = sd_device_get_property_value(dev, "ID_NAME", &name);
  if (res >= 0)
    dev_attrs.id_name = (char *)name;

  const char *uuid;
  res = sd_device_get_property_value(dev, "ID_SERIAL", &uuid);
  if (res >= 0) {
    dev_attrs.uuid = (char *)uuid;
  } else {
    res = sd_device_get_property_value(dev, "ID_FS_UUID", &uuid);
    if (res >= 0) {
      dev_attrs.uuid = (char *)uuid;
    } else {
      res = sd_device_get_property_value(dev, "ID_PART_TABLE_UUID", &uuid);
      if (res >= 0)
        dev_attrs.uuid = (char *)uuid;
    }
  }

  const char *size;
  res = sd_device_get_sysattr_value(dev, "size", &size);
  if (res >= 0)
    dev_attrs.size = (char *)size;

  const char *block_size_part_table_type;
  res = sd_device_get_property_value(dev, "ID_FS_BLOCKSIZE",
                                     &block_size_part_table_type);
  if (res >= 0)
    dev_attrs.block_size_part_table_type = (char *)block_size_part_table_type;
  else if (sd_device_get_property_value(
               dev, "ID_PART_TABLE_TYPE:", &block_size_part_table_type) >= 0)
    dev_attrs.block_size_part_table_type = (char *)block_size_part_table_type;

  return dev_attrs;
}

static int uid_is_in_list(char *device_uid) {
  FILE *fp;
  char line[1024];

  // TODO Error handle this properly
  fp = fopen(".cubbyconfig", "r");
  if (fp == NULL) {
    printf("Could not open file\n");
    return 0;
  }

  while (fgets(line, sizeof line, fp) != NULL) {
    // TODO
    // Not an ideal way of handling this
    // But for now, just check if each line exists in
    // our device uid listImages

    // REMOVE NEW LINE
    line[strcspn(line, "\n")] = 0;
    if (strcmp(device_uid, line) == 0)
      return 0;
  }

  fclose(fp);
  return -1;
}

void get_device_uid(device_attrs dev_attrs, char *device_uid) {
  // Print all details of device
  printf("\n-- Device Connected --\n");
  printf("\tName: %s\n", dev_attrs.id_name);
  printf("\tUUID: %s\n", dev_attrs.uuid);
  printf("\tSize: %s\n", dev_attrs.size);
  printf("\tBlock Size: %s\n", dev_attrs.block_size_part_table_type);
  printf("\tSyspath: %s\n\n", dev_attrs.syspath);

  // We don't want to add size, as size can change depending on the SD card
  // connected to the actual card reader
  sprintf(device_uid, "%s:%s:%s", dev_attrs.id_name, dev_attrs.uuid,
          dev_attrs.block_size_part_table_type);
}

void fail(const char *message) {
  fprintf(stderr, "FAIL: %s\n", message);
  exit(EXIT_FAILURE);
}

int device_event_handler(sd_device_monitor *monitor, sd_device *device,
                         void *userdata) {

  printf("BEGIN HANDLER\n");
  sd_device_action_t action;
  sd_device_get_action(device, &action);

  char *device_uid;
  device_attrs dev_attrs = get_sdcard_attributes(device);
  size_t device_uid_length = strlen(dev_attrs.id_name) +
                             strlen(dev_attrs.uuid) +
                             strlen(dev_attrs.block_size_part_table_type) + 4;
  device_uid = (char *)malloc(device_uid_length);

  get_device_uid(dev_attrs, device_uid);
  printf("Device UID: %s\n", device_uid);
  printf("Device Size: %s\n", dev_attrs.size);

  int device_size = atoi(dev_attrs.size);
  if (device_size == 0) {
    printf("Skip: Device has no storage attached.\n");
    return 0;
  }

  if (uid_is_in_list(device_uid) == 0) {
    printf("UID is in the LIST\n");

    const char *dev_name;
    sd_device_get_devname(device, &dev_name);

    printf("DEV NAME = %s\n", dev_name);
    mount_device_to_fs(dev_name);
  }

  return 0;
}

int setup_udev_monitoring() {
  int r;
  sd_device_monitor *monitor = NULL;
  r = sd_device_monitor_new(&monitor);
  if (r < 0)
    fail("Could not create new sd device monitor");

  r = sd_device_monitor_filter_add_match_subsystem_devtype(monitor, SUBSYSTEM,
                                                           DEVICE_TYPE);
  if (r < 0)
    fail("Could not add subsystem match to monitor");

  r = sd_device_monitor_start(monitor, device_event_handler, NULL);
  if (r < 0)
    fail("Could not start monitor");

  sd_event *event = sd_device_monitor_get_event(monitor);
  r = sd_event_loop(event);
  if (r < 0)
    fail("Could not start event loop");

  return 0;
}
