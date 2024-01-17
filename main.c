#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-device.h>
#include <unistd.h>

#define NAME    "cubby"
#define VERSION "0.1.0"

#define BOLD_TEXT    "\e[1m"
#define NO_BOLD_TEXT "\e[m"

#define SUBSYSTEM "block"
#define SD_DEV_TYPE "disk"

typedef struct {
  char *command;
  int verbose;
  int usb_device_id;
  char *backup_path;
} options_t;

void usage() {
  printf(BOLD_TEXT "OVERVIEW\n" NO_BOLD_TEXT
         "  Cubby is a small CLI utility that can listen for a drive to "
         "connect and automatically backup the contents.\n"
         "\n"
         BOLD_TEXT "USAGE\n" NO_BOLD_TEXT
         "  cubby <command> [--flags]\n"
         "\n"
         "EXAMPLES\n"
         "  cubby list\n"
         "  cubby start [-v verbose] [-i USB Device ID] [-b Backup Path]\n"
         "\n"
         "COMMANDS\n"
         "  list:   lists the available devices and their IDs to listen for\n"
         "  start:  starts cubby process to listen for device to backup \n"
         "\n"
         "GLOBAL FLAGS\n"
         "  --help, -h         Help menu\n"
         "  --verbose, -v      Enable verbose mode\n"
         "  --version          Show current version\n"
         "  --device-id        USB Device ID (start command)\n"
         "  --backup-path      File path to backup the contents of the USB "
         "drive too (start command)\n"
         "\n");
}

void fail(const char* message) {
   fprintf(stderr, "FAIL: %s\n", message);
   exit(EXIT_FAILURE);
}

int print_device_list() {
  fprintf(stderr, "No longer implemented\n");
  return -1;
}

void parse_args(int argc, char **argv, options_t *opts) {
  int opt;

  while ((opt = getopt(argc, argv, "vi:b:h")) != -1) {
    switch (opt) {
    case 'v':
      opts->verbose = 1;
      break;
    case 'i':
      opts->usb_device_id = atoi(optarg);
      break;
    case 'b':
      opts->backup_path = optarg;
      break;
    case 'h':
      usage();
      exit(EXIT_SUCCESS);
    }
  }
  // Handle positional arguments
  if (optind < argc) {
    opts->command = argv[optind];
  }

  if (opts->verbose) {
    printf("command: %s, verbose: %d, usb_id: %d, backup_path: %s\n",
           opts->command, opts->verbose, opts->usb_device_id,
           opts->backup_path);
  }
}

int device_event_handler(sd_device_monitor *monitor, sd_device *device,
                         void *userdata) {
  int r;
  const char *devname;
  const char *sysname;

  sd_device_get_devname(device, &devname);
  sd_device_get_sysname(device, &sysname);

  printf("Device event, sysname: %s, devname: %s\n", sysname, devname);

  return 1;
}

int setup_udev_monitoring() {
  int r;
  sd_device_monitor *monitor = NULL;
  r = sd_device_monitor_new(&monitor);
  if (r < 0) fail("Could not create new sd device monitor");

  r = sd_device_monitor_filter_add_match_subsystem_devtype(monitor, SUBSYSTEM, SD_DEV_TYPE);
  if (r < 0) fail("Could not add subsystem match to monitor");

  r = sd_device_monitor_start(monitor, device_event_handler, NULL);
  if (r < 0) fail("Could not start monitor");

  sd_event *event = sd_device_monitor_get_event(monitor);
  r = sd_event_loop(event);
  if (r < 0) fail("Could not start event loop");

  return 0;
}

int run_command(options_t *opts) {
  // List devices is the default command
  if (opts->command == NULL || strcmp(opts->command, "help") == 0) {
    usage();
  } else if (strcmp(opts->command, "list") == 0) {
    print_device_list();
  } else if (strcmp(opts->command, "start") == 0) {
    printf("Server Start...\n");
    setup_udev_monitoring();
    // Does not reach here
    printf("After Start...\n");
  } else {
    fprintf(stderr, "Command '%s' is not a valid command.\n", opts->command);
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  options_t opts = {NULL, 0, 0, NULL};
  parse_args(argc, argv, &opts);

  run_command(&opts);

  return EXIT_SUCCESS;
}
