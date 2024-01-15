#include "libusb-1.0/libusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  char *command;
  int verbose;
  int usb_device_id;
  char *backup_path;
} options_t;

void usage() {
  printf("Usage: cubby [-v] [-i USB Device ID] [-b Backup Path]\n");
  printf("----------------------------------------------------\n");
  printf("Options:\n");
  printf(" -v Enable verbose mode\n");
  printf(" -i USB Device ID\n");
  printf(" -b File path to backup the contents of the USB drive too\n");
  printf(" -h Help menu\n");
}

int print_device_list(libusb_context *ctx) {
  int err;
  libusb_device **list;
  ssize_t cnt = libusb_get_device_list(ctx, &list);
  if (cnt < 0) {
    fprintf(stderr, "Failed to get device list: %s\n", libusb_error_name(cnt));
    libusb_exit(ctx);
    return -1;
  }

  printf("[Device List]\n");
  // Loop over devices
  for (ssize_t i = 0; i < cnt; i++) {
    libusb_device *device = list[i];
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(list[i], &desc);

    // Check index of Product string description
    if (desc.iProduct > 0) {
      libusb_device_handle *handle;
      err = libusb_open(device, &handle);
      if (err < 0) {
        fprintf(stderr, "Error opening device: %s\n", libusb_error_name(err));
        continue;
      }
      // Read the string desc starting at iProduct index
      unsigned char product[256];
      err = libusb_get_string_descriptor_ascii(handle, desc.iProduct, product,
                                               sizeof(product));
      if (err > 0) {
        printf("Device %04x:%04x %s\n", desc.idVendor, desc.idProduct, product);
      } else {
        fprintf(stderr, "Failed to get device description: %s\n",
                libusb_error_name(err));
      }
      libusb_close(handle);
    }
  }
  libusb_free_device_list(list, 1);
  return 0;
}

void print_hotplug_cap() {
  int has_hotplug = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG);
  if (has_hotplug > 0) {
    printf("Hotplug AVAILABLE\n");
  } else {
    printf("Hotplug is not available on this machine, exiting.\n");
  }
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

int hotplug_callback(libusb_context *ctx, libusb_device *device,
                     libusb_hotplug_event event, void *user_data) {
  switch (event) {
  case LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED:
    printf("Target device arrived\n");
    break;
  case LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT:
    printf("Target device left\n");
    break;
  default:
    printf("Unknown event\n");
    break;
  }

  return 0;
}

void register_hotplug_listener() {}

int run_command(libusb_context *ctx, options_t *opts) {
  // List devices is the default command
  if (opts->command == NULL || strcmp(opts->command, "list") == 0) {
    print_device_list(ctx);
  } else if (strcmp(opts->command, "start") == 0) {
    printf("Server Start...\n");
  } else {
    fprintf(stderr, "Command '%s' is not a valid command.", opts->command);
    return EXIT_FAILURE;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  options_t opts = {NULL, 0, 0, NULL};
  parse_args(argc, argv, &opts);

  libusb_context *ctx = NULL;
  int err = libusb_init(&ctx);
  if (err < 0) {
    fprintf(stderr, "Failed to initialize libusb: %s\n",
            libusb_error_name(err));
    return EXIT_FAILURE;
  }

  if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
    fprintf(stderr, "Hotplug capability is not supported by this platform.\n");
    return EXIT_FAILURE;
  }

  run_command(ctx, &opts);

  libusb_exit(ctx);
  return EXIT_SUCCESS;
}
