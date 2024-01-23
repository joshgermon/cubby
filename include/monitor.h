#ifndef MONITOR_H
#define MONITOR_H

#include <systemd/sd-device.h>

int setup_udev_monitoring();
int get_list_of_available_devices();
const char* get_largest_partition(const char *devpath);

#endif /* MONITOR_H */
