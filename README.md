# Cubby (C USB Backup Buddy)

## Overview
A small CLI utility that listens for a specific USB event (mounting/insert) and attempts to backup the drive to a specified location.

## Ideas
- CLI interface to interact with
- CLI interface to start a server
- Specify the drive to listen for --usb-device / -d
- Specify the location to backup too --backup-dir -b
- CLI can be used in client mode to connect to a running server
- Server writes logs to a file
- Client can connect and read logs
- Client parses log format possibly into nice status updates
- Headless mode (run in background)

Example Server CLI command:
`cubby --mode server --usb <identifier> --backup-dir ./backup`

Interactively pick USB device
```
cubby ls
[1] USB ABC
[2] USB ABC
```
