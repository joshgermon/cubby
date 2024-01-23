#include "../include/backup.h"
#include "systemd/sd-device.h"
#include <blkid/blkid.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUBSYSTEM "block"
#define DEVICE_TYPE "partition"

typedef struct {
  char *id_name;
  char *uuid;
  char *size;
  char *block_size_part_table_type;
  char *syspath;
} device_attrs;

const char* get_largest_partition(const char *devpath) {
  static char largest_part_devname[16];
  blkid_probe probe = blkid_new_probe_from_filename(devpath);
  if (!probe) {
    fprintf(stderr, "Error creating probe: %s\n", strerror(errno));
    return NULL;
  }

  // Get number of partitions
  blkid_partlist part_list;
  int part_count, i;

  part_list = blkid_probe_get_partitions(probe);
  part_count = blkid_partlist_numof_partitions(part_list);

  if (part_count <= 0) {
    printf("Could not find any partitions for given device.\n");
    return NULL;
  }

  blkid_partition largest_partition = NULL;
  unsigned long long max_size = 0;

  // Loop over partitions to find the largest size
  for (i = 0; i < part_count; i++) {
    blkid_partition part = blkid_partlist_get_partition(part_list, i);
    unsigned long long size = blkid_partition_get_size(part);

    printf("Part %s%d is %llu\n", devpath, blkid_partition_get_partno(part), size);

    if (size > max_size) {
      max_size = size;
      largest_partition = part;
    }
  }

  snprintf(largest_part_devname, sizeof(largest_part_devname), "%s%d", devpath, blkid_partition_get_partno(largest_partition));
  printf("Largest part is %s\n", largest_part_devname);

  blkid_free_probe(probe);
  return largest_part_devname;
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

    // Find largest partition of drive
    const char *part_dev_name = get_largest_partition(dev_name);
    // Mount partition to file system
    mount_device_to_fs(part_dev_name);
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

int get_list_of_available_devices() {
  sd_device_enumerator *sd_dev_enum;
  sd_device_enumerator_new(&sd_dev_enum);

  sd_device *current_device;
  sd_device_enumerator_add_match_subsystem(sd_dev_enum, "block", 1);

  for (current_device = sd_device_enumerator_get_device_first(sd_dev_enum);
       current_device != NULL;
       current_device = sd_device_enumerator_get_device_next(sd_dev_enum)) {
    const char *id_model = NULL;
    int r = sd_device_get_property_value(current_device, "ID_MODEL", &id_model);
    if (r >= 0) {
      printf("%s\n", id_model);
      device_attrs dev_attrs = get_sdcard_attributes(current_device);
      printf("%s:%s:%s", dev_attrs.id_name, dev_attrs.uuid, dev_attrs.syspath);
    }
    sd_device_unref(current_device);
  }

  sd_device_enumerator_unref(sd_dev_enum);
  return 0;
}

