#ifndef BACKUP_H
#define BACKUP_H

#define SUBSYSTEM "block"
#define DEVICE_TYPE "partition"

#include "cubby.h"

typedef struct {
  char *id_name;
  char *uuid;
  char *size;
  char *block_size_part_table_type;
  char *syspath;
} device_attrs;

int backup_dir(cubby_opts_t *opts, const char *source_dir);
const char* mount_device_to_fs(const char *dev_name);
void unmount_device(const char *mount_path);

#endif /* BACKUP_H */
