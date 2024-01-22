#ifndef MONITOR_H
#define MONITOR_H

#include <systemd/sd-device.h>

int setup_udev_monitoring();
int get_list_of_available_devices();

#endif /* MONITOR_H */
