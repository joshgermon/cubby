# Cubby (C USB Backup Buddy)

## Overview
A command line tool / Linux daemon written in C with minimal dependencies that listens for a specific USB event (mounting/insert) and attempts to backup the drive to a specified location (using rsync).

The ultimate goal of this application was born out of a wish to automatically backup files from an SD Card once inserted into a USB SD Card Reader without user intervention.

## Platform

Cubby is written for Linux platforms utilising the systemd API (lib-systemd) to listen for device events with udev. As device event monitoring is very much OS specific, this will not work for non-Linux platforms or Linux installations without systemd.

## Usage

Example CLI usage

```
cubby <command> [--flags]
cubby list
cubby start [-i USB Device ID] [-b Backup Path]
```

## Planning

### Todo List

- [x] Successfully listens for device events
- [x] Mounts partition of device to filesystem
- [x] Accepts flags / options
- [x] Backs up trusted device to provided backup path with Rsync
- [x] Provide way to list devices
- [x] Provide way to interactively pick trusted device from listed devices
- [x] Make a new directory with timestamp to backup too i.e ./backup/2023-01-01_1
- [x] Improved validation of flags and commands i.e X flag only valid for Y command
- [x] Implement list devices command
- [x] Early exit on (non-root user) missing permissions to mount
- [x] Fix broken CLI options parsing
- [x] Improve CLI options for per command parsing
- [x] Improve/simplify error handling i.e die()
- [ ] Config file / config pass along includes to rsync i.e. *.ARW
- [ ] Review UID for devices and if they are too specific (specific to SD size?)
- [ ] Report on new events / status / progress to some network
- [ ] Backup specific folder on device if it exists (DCIM/)
- [ ] Fine grain permission check on missing permissions to mount
- [ ] Run as a service investigation
- [ ] Investigate behaviour when multiple partitions are added in an event (possible duplicate runs)
- [ ] General logging
- [ ] Prettify the CLI interactions


### Initial Sketch-out of Ideas

- CLI interface to interact with
- CLI interface to start a server
- Specify the drive to listen for --usb-device / -d
- Specify the location to backup too --backup-dir -b
- CLI can be used in client mode to connect to a running server
- Server writes logs to a file
- Client can connect and read logs
- Client parses log format possibly into nice status updates
- Headless mode (run in background)
- Send notifications via slack webhooks (or other) of progress
- Create folders based on latest modified date of file
- Check if already backed up? Needs to be quick enough. Quick sanity check? Option to ignore.

Example Server CLI command:
`cubby --mode server --usb <identifier> --backup-dir ./backup`
`cubby start --usb <identifier> --backup-dir ./backup`

Interactively pick USB device
```
cubby ls
[1] USB ABC
[2] USB ABC
```

### Note on libusb hotplug to udev rules (changing initial approach)

Initially had approached testing by inserting and removing a USB Storage Drive and using the libusb library, use the hotplug functionality to listen for the specified drive's connection and disconnection.

This approach was successful with a USB storage drive being physically inserted and removed, however the actual goal of this project was to have a Media Card Reader (SD Card Reader) already inserted into a USB port on a machine and listen for SD Cards being inserted into that card reader. This is *not* a USB event, therefore libusb is not a library that can help here.

The reason it was chosen in the first place was the cross-platform nature of the library rather than the other initial idea of using udev rules/events.

In further testing of inserting/removing SD cards while using an USB SD Card Reader monitoring udev confirmed the correct events could be observed here.

In terms of a C library to interact with udev, there is libudev, however according to the documentation - it's no longer recommended for new projects. Instead sd-device API is a part of the systemd library which is a more "modern" replacement. The only issue with this is there is no clear documentation on it, really anywhere.

Although looking in the header file reveals the functionality is very close to the `libudev` API it replaced, so that should help.


