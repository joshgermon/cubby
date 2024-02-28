#include "../include/cubby.h"
#include "../include/utils.h"
#include <getopt.h>
#include <string.h>

static struct option default_opts[] = {
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

static struct option start_opts[] = {
    {"verbose", no_argument, 0, 'v'},
    {"backup-path", required_argument, 0, 'b'},
    {0, 0, 0, 0}
};

static void parse_default_args(int argc, char **argv, cubby_opts_t *opts) {
  int opt;
  int opt_index = 0;

  while ((opt = getopt_long(argc, argv, "v", default_opts, &opt_index)) != -1) {
    switch (opt) {
    case 'v':
      opts->verbose = 1;
      break;
    default:
      die("Invalid option provided for this command.\n");
    }
  }
}

static void parse_start_opts(int argc, char **argv, cubby_opts_t *opts) {
  int opt;
  int opt_index = 0;

  while ((opt = getopt_long(argc, argv, "vb:", start_opts, &opt_index)) != -1) {
    switch (opt) {
    case 'v':
      opts->verbose = 1;
      break;
    case 'b':
      opts->backup_path = optarg;
      break;
    default:
      die("Invalid option provided for this command.\n");
    }
  }

  if (opts->backup_path == NULL) die("Backup path is required.\n");
}

void parse_args(int argc, char **argv, cubby_opts_t *opts) {
  opts->command = argv[1];
  if (opts->command == NULL) return;

  if (strcmp(opts->command, "list") == 0) {
    parse_default_args(argc, argv, opts);
  } else if (strcmp(opts->command, "help") == 0) {
    parse_default_args(argc, argv, opts);
  } else if (strcmp(opts->command, "start") == 0) {
    parse_start_opts(argc, argv, opts);
  } else {
    die("Command is not a valid command, run 'cubby help' to see valid usage.");
  }
}
