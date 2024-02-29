#include "../include/cli.h"
#include "../include/cubby.h"
#include "../include/monitor.h"
#include "../include/utils.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage()
{
    printf(BOLD_TEXT
    "OVERVIEW\n" NO_BOLD_TEXT
    "  Cubby is a small CLI utility that can listen for a drive to "
    "connect and automatically backup the contents.\n"
    "\n" BOLD_TEXT "USAGE\n" NO_BOLD_TEXT "  cubby <command> [--flags]\n"
    "\n" BOLD_TEXT "EXAMPLES\n" NO_BOLD_TEXT "  cubby list\n"
    "  cubby start [-i USB Device ID] [-b Backup Path]\n"
    "\n"
    "COMMANDS\n" NO_BOLD_TEXT
    "  list:   lists the available devices and their IDs to listen for\n"
    "  start:  starts cubby process to listen for device to backup\n"
    "  help:   prints this help menu and usage guide\n"
    "\n"
    "GLOBAL FLAGS\n"
    "  --verbose, -v      Enable verbose mode\n"
    "\n");
}

int run_command(cubby_opts_t *opts)
{
    /** List devices is the default command */
    if (opts->command == NULL || strcmp(opts->command, "help") == 0) {
        usage();
    } else if (strcmp(opts->command, "list") == 0) {
        print_device_list();
    } else if (strcmp(opts->command, "start") == 0) {
        /* Backup path is required */
        if (!opts->backup_path)
            die("No backup path provided.");

        /* If no device id provided, prompt user for one */
        if (opts->device_uid == NULL)
            opts->device_uid = ask_user_for_trusted_device();

        setup_udev_monitoring(opts);
    } else {
        die("Command is not a valid command, run 'cubby help' to see "
            "valid usage.\n");
    }
    exit(EXIT_SUCCESS);
}

void ensure_perms()
{
    /** Check for root user */
    /** TODO: Update to more fine-grained permissions around mounting devices */
    if (getuid() != 0)
        die("This program is required to run as root to mount and create "
            "directories. "
            "Please try again as root.\n");
}

int main(int argc, char *argv[])
{
    cubby_opts_t opts = { NULL, 0, NULL, NULL, NULL, NULL, 0 };

    ensure_perms();
    parse_args(argc, argv, &opts);
    run_command(&opts);

    return EXIT_SUCCESS;
}
