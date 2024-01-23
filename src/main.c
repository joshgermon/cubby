#include "../include/monitor.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "cubby"
#define VERSION "0.1.0"

#define BOLD_TEXT "\e[1m"
#define NO_BOLD_TEXT "\e[m"

typedef struct {
  char *command;
  int verbose;
  int usb_device_id;
  char *backup_path;
} options_t;

void usage() {
  printf(BOLD_TEXT
         "OVERVIEW\n" NO_BOLD_TEXT
         "  Cubby is a small CLI utility that can listen for a drive to "
         "connect and automatically backup the contents.\n"
         "\n" BOLD_TEXT "USAGE\n" NO_BOLD_TEXT "  cubby <command> [--flags]\n"
         "\n" BOLD_TEXT "EXAMPLES\n" NO_BOLD_TEXT "  cubby list\n"
         "  cubby start [-v verbose] [-i USB Device ID] [-b Backup Path]\n"
         "\n"
         "COMMANDS\n" NO_BOLD_TEXT
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

int print_device_list() {
  char *largest_part_devname[9];
  get_largest_partition("/dev/sdc", &largest_part_devname);
  return 0;
}

static struct option long_options[] = {
    {"list", required_argument, 0, 'l'},
    {"backup-path", required_argument, 0, 'b'},
    {"device-id", no_argument, 0, 'i'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'c'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}};

void parse_args(int argc, char **argv, options_t *opts) {
  int opt;
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "vsi:b:h", long_options,
                            &option_index)) != -1) {
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
