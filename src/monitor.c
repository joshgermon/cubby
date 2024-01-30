#include "../include/cubby.h"
#include "../include/monitor.h"
#include "../include/backup.h"
#include "systemd/sd-device.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DeviceAttributes get_sdcard_attributes(sd_device *dev) {
  DeviceAttributes dev_attrs = {"noname", "noserial", "nosize",
                            "noblocksize", "nopath"};
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

  const char *block_size;
  res = sd_device_get_property_value(dev, "ID_FS_BLOCKSIZE",
                                     &block_size);
  if (res >= 0)
    dev_attrs.block_size = (char *)block_size;
  else if (sd_device_get_property_value(
               dev, "ID_PART_TABLE_TYPE:", &block_size) >= 0)
    dev_attrs.block_size = (char *)block_size;

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

    // Remove new line
    line[strcspn(line, "\n")] = 0;
    if (strcmp(device_uid, line) == 0)
      return 0;
  }

  fclose(fp);
  return -1;
}

static void print_event_log(DeviceAttributes dev_attrs) {
  // Print all details of device
  printf("\n-- Trusted Device Event --\n");
  printf("\tName: %s\n", dev_attrs.id_name);
  printf("\tUUID: %s\n", dev_attrs.uuid);
  printf("\tSize: %s\n", dev_attrs.size);
  printf("\tBlock Size: %s\n", dev_attrs.block_size);
  printf("\tSyspath: %s\n\n", dev_attrs.syspath);
}

void get_device_uid(DeviceAttributes dev_attrs, char *device_uid, size_t device_uid_len) {
  // We don't want to add size, as size can change depending on the SD card
  // connected to the actual card reader
  snprintf(device_uid, device_uid_len, "%s:%s:%s", dev_attrs.id_name, dev_attrs.uuid,
          dev_attrs.block_size);
}

void fail(const char *message) {
  fprintf(stderr, "FAIL: %s\n", message);
  exit(EXIT_FAILURE);
}

int device_event_handler(sd_device_monitor *monitor, sd_device *device,
                         void *userdata) {
  // Retrieve cubby options from userdata
  cubby_opts_t *opts = (cubby_opts_t*)userdata;

  sd_device_action_t action;
  sd_device_get_action(device, &action);

  char *device_uid;
  DeviceAttributes dev_attrs = get_sdcard_attributes(device);
  size_t device_uid_length = strlen(dev_attrs.id_name) +
                             strlen(dev_attrs.uuid) +
                             strlen(dev_attrs.block_size) + 3;
  device_uid = (char *)malloc(device_uid_length);

  get_device_uid(dev_attrs, device_uid, device_uid_length);

  // Return if device is not a trusted device
  if (strcmp(device_uid, opts->usb_device_id) != 0) {
    printf("Ignoring event, device is not a *trusted device*.\n");
    return 0;
  }

  print_event_log(dev_attrs);

  int device_size = atoi(dev_attrs.size);
  if (device_size == 0) {
    printf("Skip: Device has no storage attached.\n");
    return 0;
  }

  const char *dev_name;
  sd_device_get_devname(device, &dev_name);

  // Mount partition to file system and backup
  const char* mount_path = mount_device_to_fs(dev_name);
  backup_dir(opts, mount_path);

  unmount_device(mount_path);
  free(device_uid);
  return 0;
}

int setup_udev_monitoring(cubby_opts_t *opts) {
  int r;
  sd_device_monitor *monitor = NULL;
  r = sd_device_monitor_new(&monitor);
  if (r < 0)
    fail("Could not create new sd device monitor");

  r = sd_device_monitor_filter_add_match_subsystem_devtype(monitor, SUBSYSTEM,
                                                           DEVICE_TYPE);
  if (r < 0)
    fail("Could not add subsystem match to monitor");

  r = sd_device_monitor_start(monitor, device_event_handler, opts);
  if (r < 0)
    fail("Could not start monitor");

  sd_event *event = sd_device_monitor_get_event(monitor);
  r = sd_event_loop(event);
  if (r < 0)
    fail("Could not start event loop");

  return 0;
}

DeviceList new_device_list() {
  sd_device_enumerator *sd_dev_enum;
  sd_device_enumerator_new(&sd_dev_enum);

  // Create initial buffer
  int dev_buffer_size = 16;
  char **devices = malloc(sizeof(*devices) * dev_buffer_size);
  if (devices == NULL) {
    exit(EXIT_FAILURE);
  }

  sd_device *current_device;
  sd_device_enumerator_add_match_subsystem(sd_dev_enum, "block", 1);

  int i = 0;
  for (current_device = sd_device_enumerator_get_device_first(sd_dev_enum);
       current_device != NULL;
       current_device = sd_device_enumerator_get_device_next(sd_dev_enum)) {

    // Get current device unique ID
    char *device_uid;
    DeviceAttributes dev_attrs = get_sdcard_attributes(current_device);
    size_t device_uid_length = strlen(dev_attrs.id_name) +
                               strlen(dev_attrs.uuid) +
                               strlen(dev_attrs.block_size) + 3;

    device_uid = (char *)malloc(device_uid_length);
    get_device_uid(dev_attrs, device_uid, device_uid_length);

    // If we hit our max initial size, reallocate more memory
    if (i >= dev_buffer_size) {
      dev_buffer_size *= 2;
      char **new_devices =
          realloc(devices, sizeof(*new_devices) * dev_buffer_size);
      if (new_devices == NULL) {
        free(devices);
        exit(EXIT_FAILURE);
      }
      devices = new_devices;
    }
    devices[i++] = device_uid;
  }

  DeviceList device_list = { .devices = devices, .length = i };

  sd_device_unref(current_device);
  sd_device_enumerator_unref(sd_dev_enum);

  return device_list;
}

void cleanup_device_list(DeviceList device_list) {
  for(int i = 0; i < device_list.length; i++) {
    free(device_list.devices[i]);
  }
  free(device_list.devices);
}

int print_device_list() {
  DeviceList device_list = new_device_list();

  printf("--- Available Devices ---\n");
  for(int i = 0; i < device_list.length; i++) {
    printf("  [%d]\t %s\n", i, device_list.devices[i]);
  }
  printf("\n");

  cleanup_device_list(device_list);
  return 0;
}

char *ask_user_for_trusted_device() {
  int selection;
  DeviceList device_list = new_device_list();
  print_device_list();

  printf("Choose device to listen for: ");
  scanf("%d", &selection);

  static char selected_device[64];
  strncpy(selected_device, device_list.devices[selection], 64);

  printf("Your chosen device: %s\n", selected_device);

  cleanup_device_list(device_list);
  return selected_device;
}
