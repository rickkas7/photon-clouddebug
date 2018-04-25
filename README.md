# Photon and P1 Cloud Debug

*Special code for debugging cloud connection issues with the Particle Photon and P1*

## What is this?

This is a tool to debug cloud connection issues. It:

- Prints out your Wi-Fi configuration information
- Prints out available Wi-Fi access points
- Pings your IP gateway
- Pings the Google DNS (8.8.8.8)
- Does a DNS server lookup of the device server (device.spark.io)
- Makes a test connection to the CoAP port (5683) on the device server
- Makes an actual cloud connection
- Acts like Tinker after connecting 

Here's an example output log:

```
$ particle serial monitor
Opening serial monitor for com port: "/dev/cu.usbmodemFD1161"
0000008028 [system] INFO: ARM_WLAN_WD 2
0000008028 [hal.wlan] INFO: Bringing WiFi interface up with DHCP
0000010040 [system] INFO: CLR_WLAN_WD 1, DHCP success
connected to WiFi!
localIP=192.168.2.180
subnetMask=255.255.255.0
gatewayIP=192.168.2.1
dnsServerIP=0.0.0.0 (often 0.0.0.0)
dhcpServerIP=0.0.0.0 (often 0.0.0.0)
ping gateway=1
ping addr 8.8.8.8=1
device.spark.io=54.173.1.44
connected to device server CoAP (testing connection only)
connecting to cloud
0000010114 [system] INFO: Cloud: connecting
0000010118 [system] INFO: Resolved host device.spark.io to 54.173.1.44
0000010152 [system] INFO: connected to cloud 54.173.1.44:5683
0000010152 [system] INFO: Cloud socket connected
0000010505 [comm] INFO: Hanshake: completed
0000010507 [system] INFO: Cloud connected
connected to the cloud!
```

The source code is [here](https://github.com/rickkas7/photon-clouddebug/blob/master/clouddebug.cpp).

## Prerequisites 

- You should have the [Particle CLI](https://docs.particle.io/guide/tools-and-features/cli/photon/) installed
- You must have a working dfu-util or JTAG/SWD programmer

## To Install - Photon (0.5.3 or later)

Download the [clouddebug.bin](https://github.com/rickkas7/photon-clouddebug/blob/master/clouddebug.bin) file.

Put the Photon in DFU mode (blinking yellow) by holding down RESET and SETUP, releasing RESET while continuing to hold down SETUP. The main status LED will blink magenta (blue and red at the same time), then yellow. Once blinking yellow, release SETUP.

From a Command Prompt or Terminal window:

```
particle flash --usb clouddebug.bin
```

The Photon will restart. Immediately open a serial window. One easy way is to use the Particle CLI:

```
particle serial monitor
```

If you prefer to compile the binary yourself:

```
particle compile photon clouddebug.cpp --target 0.5.3 --saveTo clouddebug.bin
```

If after restart the Photon blinks magenta again, then goes through a sequence of blinking green, blinking cyan (if may not get all the way through the sequence), you may need to upgrade the device system firmware.

Put the Photon in DFU mode (blinking yellow) by holding down RESET and SETUP, releasing RESET while continuing to hold down SETUP. The main status LED will blink magenta (blue and red at the same time), then yellow. Once blinking yellow, release SETUP.

Then:

```
particle update
```

## To Install - P1 (with USB)

Download the [combined-p1.bin](https://github.com/rickkas7/photon-clouddebug/raw/master/combined-p1.bin) file.

Put the Photon in DFU mode (blinking yellow) by pressing RESET and SETUP. Release RESET and continue to hold down SETUP while the LED blinks magenta until it blinks yellow, then release SETUP.

Issue the command:

```
dfu-util -d 2b04:d008 -a 0 -s 0x8020000:leave -D combined-p1.bin
```

The Photon will restart. Immediately open a serial window. For example, using the CLI:

```
particle serial monitor
```

## To Install - P1 (using JTAG and Serial1)

Download the [combined-p1ser1.bin](https://github.com/rickkas7/photon-clouddebug/raw/master/combined-p1ser1.bin) file.

Program and verify the binary using JTAG/SWD to address 0x8020000.

Connect the TX pin to something than receive the serial data at 9600 baud 8N1.

Reset the P1. The debugging information will be sent out Serial1.

## To Remove

You can just flash a different binary over it, or restore Tinker:

Put the Photon in DFU mode (blinking yellow) by pressing RESET and SETUP. Release RESET and continue to hold down SETUP while the LED blinks magenta until it blinks yellow, then release SETUP.

```
particle flash --usb tinker
```

