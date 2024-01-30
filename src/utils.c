#include <blkid/blkid.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-device.h>

const char *get_largest_partition(const char *devpath) {
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

    printf("Part %s%d is %llu\n", devpath, blkid_partition_get_partno(part),
           size);

    if (size > max_size) {
      max_size = size;
      largest_partition = part;
    }
  }

  snprintf(largest_part_devname, sizeof(largest_part_devname), "%s%d", devpath,
           blkid_partition_get_partno(largest_partition));
  printf("Largest part is %s\n", largest_part_devname);

  blkid_free_probe(probe);
  return largest_part_devname;
}

