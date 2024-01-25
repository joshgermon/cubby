#ifndef CUBBY_H
#define CUBBY_H

#define APP_NAME "cubby"
#define VERSION "0.1.0"

#define BOLD_TEXT "\e[1m"
#define NO_BOLD_TEXT "\e[m"

typedef struct {
  char *command;
  int verbose;
  int usb_device_id;
  char *backup_path;
} cubby_opts_t;

#endif
