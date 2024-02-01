#include "../include/cubby.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static struct option valid_opts[] = { { "backup-path", required_argument, 0, 'b' },
    { "uuid", required_argument, 0, 'i' }, { 0, 0, 0, 0 } };

void parse_command_opts (int argc, char **argv, cubby_opts_t *opts)
{
    int opt;
    int option_index = 0;

    while ((opt = getopt_long (argc, argv, "b:i:", valid_opts, &option_index)) != -1) {
        switch (opt) {
        case 'i': opts->usb_device_id = optarg; break;
        case 'b': opts->backup_path = optarg; break;
        default:
            fprintf (stderr, "Unknown option '-%c'.\n", opt);
            exit (EXIT_FAILURE);
        }
    }
}

void parse_args (int argc, char **argv, cubby_opts_t *opts)
{
    // Handle positional arguments
    opts->command = argv[optind];
    parse_command_opts (argc, argv, opts);
}
