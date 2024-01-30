#ifndef MONITOR_H
#define MONITOR_H

#include "cubby.h"
#include <systemd/sd-device.h>

typedef struct {
  char *id_name;
  char *uuid;
  char *size;
  char *block_size;
  char *syspath;
} DeviceAttributes;

typedef struct {
  char **devices;
  int length;
} DeviceList;

int setup_udev_monitoring(cubby_opts_t *opts);
DeviceAttributes get_sdcard_attributes(sd_device *dev);
DeviceList new_device_list();
void cleanup_device_list(DeviceList device_list);
int print_device_list();
char* ask_user_for_trusted_device();

#endif /* MONITOR_H */
