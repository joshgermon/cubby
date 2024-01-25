#ifndef MONITOR_H
#define MONITOR_H

#include "cubby.h"
#include <systemd/sd-device.h>

typedef struct {
  char *id_name;
  char *uuid;
  char *size;
  char *block_size_part_table_type;
  char *syspath;
} device_attrs;

int setup_udev_monitoring(cubby_opts_t *opts);
int get_list_of_available_devices();
device_attrs get_sdcard_attributes(sd_device *dev);

#endif /* MONITOR_H */
