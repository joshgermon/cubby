#include "../include/cubby.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *command;
  struct option *valid_options;
} command_opts_t;

// For commands with no flags
static struct option no_options[] = {
  {0, 0, 0, 0}
};

static struct option list_options[] = {
  {"uuid", required_argument, 0, 'u'},
  {0, 0, 0, 0}
};

static struct option start_options[] = {
    {"backup-path", required_argument, 0, 'b'},
    {"uuid", no_argument, 0, 'i'},
    {0, 0, 0, 0}
};

static command_opts_t command_opts[] = {
    {"list", list_options},
    {"start", start_options},
    {"help", no_options},
    {NULL, NULL}
};

void parse_global_opts(int argc, char **argv, cubby_opts_t *opts) {
  int opt;
  int option_index = 0;

  static struct option global_options[] = {{"verbose", no_argument, 0, 'v'},
                                           {0, 0, 0, 0}};

  while ((opt = getopt_long(argc, argv, "hv", global_options, &option_index)) !=
         -1) {
    switch (opt) {
    case 'v':
      opts->verbose = 1;
      break;
    default:
      break;
    }
  }
}

struct option *get_valid_opts(char *command) {
  for (command_opts_t *cmd_opt = command_opts; cmd_opt->command != NULL;
       cmd_opt++) {
    // Find valid options for current command
    if (strcmp(command, cmd_opt->command) == 0) {
      return cmd_opt->valid_options;
    }
  }
  return NULL;
}

void parse_command_opts(int argc, char **argv, cubby_opts_t *opts,
                        struct option *valid_options) {
  int opt;
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "", valid_options, &option_index)) !=
         -1) {
    switch (opt) {
    case 'i':
      opts->usb_device_id = optarg;
      break;
    case 'b':
      opts->backup_path = optarg;
      break;
    default:
      fprintf(stderr, "Unknown option '-%c'.\n", opt);
      exit(EXIT_FAILURE);
    }
  }
}

void parse_args(int argc, char **argv, cubby_opts_t *opts) {
  parse_global_opts(argc, argv, opts);

  // Handle positional arguments
  if (optind < argc) {
    opts->command = argv[optind];
    struct option *valid_opts = get_valid_opts(opts->command);
    if (valid_opts == NULL) {
      fprintf(stderr, "Command '%s' is not a valid command, use 'cubby help' to see valid usage.\n", opts->command);
      exit(EXIT_FAILURE);
    }
    parse_command_opts(argc, argv, opts, valid_opts);
  }

  if (opts->verbose) {
    printf("command: %s, verbose: %d, usb_id: %d, backup_path: %s\n",
           opts->command, opts->verbose, opts->usb_device_id,
           opts->backup_path);
  }
}
