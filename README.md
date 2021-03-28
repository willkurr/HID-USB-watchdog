# HID-USB-watchdog
Allows a standard HID USB watchdog to be used on a Linux machine.

A popular way to monitor Linux machines are cheap little USB watchdogs which listen for a heartbeat from the host, and connect directly to the power and reset button headers on the motherboard. They have two relays for jumping these pins and resetting the machine in case, for example, a kernel panic occured and the heartbeat is no longer being sent. In the case of Windows users, these watchdogs typically ship with an executable which does all the work for you. Unfortunately for those on Linux, this isn't the case, and you need to write your own code to do this. 

These USB watchdogs in question act as an HID device, and just accept packets of raw bytes sent to it. It accepts packets of 2 bytes or more, otherwise the transmit fails. In particular, the first byte is the restart delay in seconds divide by 10. This determines how long the watchdog should reset the rig after if a heartbeat is not recieved. After recieving the packet, it sends back a packet of equal size and value, confirming it recieved what was sent.

## Building
To build, the [hidapi library](https://github.com/libusb/hidapi) is needed. You can follow their instructions to build on Linux or install it with apt by doing

`sudo apt install libhidapi-hidraw0 libhidapi-libusb0` (be cautious with this, I built using their instructions so this may not work. Leaving it here incase it does by some chance)

Then you just need to `make` and the executable will be built and named as `hidusbwatchdog`. 

## Usage
To run, do the following with root privileges:

`hidusbwatchdog heartbeat_interval refresh_rate`

`heartbeat_interval` is a number from 0-2550 seconds which tells the watchdog how long to wait after not recieving a heartbeat to reset the machine. This needs to be in increments of 10 seconds since the watchdog only accepts such values. The program divides this value you provide by 10, so anything in the ones place will always truncate. Be careful not to set this too low, otherwise your machine may get stuck in an infinite reboot loop! Idealy you want to set this just a little longer than it takes the machine to boot and start this program.

`refresh_rate` is a number in seconds which specifies how often the machine sends a heartbeat to the watchdog. It must be less than `heartbeat_interval`. (obviously)
