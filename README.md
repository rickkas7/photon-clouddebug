# Photon Cloud Debug

*Special code for debugging cloud connection issues with the Particle Photon*

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

Meanwhile, it uses a special debug version of the system firmware, so there's additional debugging information generated as well.

Here's an example output log:

```
$ particle serial monitor
Opening serial monitor for com port: "/dev/cu.usbmodemFD1161"
configured credentials:
ssid=TestNet security=wpa2 cipher=1
available access points:
SSID=TestNet security=wpa2 channel=1 rssi=-66
connecting to WiFi
0000007054:INFO : virtual void ManagedNetworkInterface::connect(bool) (282):ready():false,connecting():false,listening():false
connected to WiFi!
localIP=192.168.2.180
subnetMask=255.255.255.0
gatewayIP=192.168.2.1
dnsServerIP=0.0.0.0 (often 0.0.0.0)
dhcpServerIP=0.0.0.0 (often 0.0.0.0)
ping gateway=1
ping addr 8.8.8.8=1
device.spark.io=52.91.48.237
0000011078:DEBUG: virtual void TCPClient::stop() (192):_sock -1 closesocket
0000011078:DEBUG: virtual int TCPClient::connect(IPAddress, uint16_t, network_interface_t) (80):socket=536895952
0000011079:DEBUG: virtual int TCPClient::connect(IPAddress, uint16_t, network_interface_t) (98):_sock 536895952 connect
0000011112:DEBUG: virtual int TCPClient::connect(IPAddress, uint16_t, network_interface_t) (100):_sock 536895952 connected=1
connected to device server CoAP (testing connection only)
0000011113:DEBUG: virtual void TCPClient::stop() (192):_sock 536895952 closesocket
0000011114:DEBUG: sock_result_t socket_close(sock_handle_t) (939):socket closed 200061d0
connecting to cloud
0000011114:INFO : void establish_cloud_connection() (214):Cloud: connecting
0000011115:DEBUG: int spark_cloud_socket_connect() (834):sparkSocket Now =-1
0000011115:DEBUG: int spark_cloud_socket_connect() (853):HAL_FLASH_Read_ServerAddress() = type:1,domain:device.spark.io,ip: 105.118.101.100, port: 65535
0000011119:INFO : int determine_connection_address(IPAddress&, uint16_t&, ServerAddress&, bool) (803):Resolved host device.spark.io to 52.91.48.237
0000011119:DEBUG: int spark_cloud_socket_connect() (864):socketed udp=0, sparkSocket=536895672, 1
0000011120:DEBUG: int spark_cloud_socket_connect() (874):connection attempt to 52.91.48.237:5683
0000011157:INFO : int spark_cloud_socket_connect() (891):connected to cloud 52.91.48.237:5683
0000011157:INFO : void establish_cloud_connection() (221):Cloud socket connected
0000011158:DEBUG: int Spark_Handshake(bool) (546):starting handshake announce=1
0000011189:DEBUG: int read_packet_and_dispose(tcp_packet_t&, void*, int, wiced_tcp_socket_t*, int) (792):Socket 0 receive bytes 40 of 40
0000011237:DEBUG: sock_result_t socket_send(sock_handle_t, const void*, socklen_t) (1002):Write 256 bytes to socket 536895672 result=0
0000011276:DEBUG: int read_packet_and_dispose(tcp_packet_t&, void*, int, wiced_tcp_socket_t*, int) (792):Socket 0 receive bytes 384 of 384
0000011466:DEBUG: sock_result_t socket_send(sock_handle_t, const void*, socklen_t) (1002):Write 18 bytes to socket 536895672 result=0
0000011468:DEBUG: sock_result_t socket_send(sock_handle_t, const void*, socklen_t) (1002):Write 18 bytes to socket 536895672 result=0
0000011500:DEBUG: int read_packet_and_dispose(tcp_packet_t&, void*, int, wiced_tcp_socket_t*, int) (792):Socket 0 receive bytes 2 of 18
0000011501:DEBUG: int read_packet_and_dispose(tcp_packet_t&, void*, int, wiced_tcp_socket_t*, int) (792):Socket 0 receive bytes 16 of 16
0000011502:INFO : int SparkProtocol::handshake() (123):Hanshake: completed
0000011503:DEBUG: sock_result_t socket_send(sock_handle_t, const void*, socklen_t) (1002):Write 50 bytes to socket 536895672 result=0
0000011503:DEBUG: sock_result_t socket_send(sock_handle_t, const void*, socklen_t) (1002):Write 50 bytes to socket 536895672 result=0
0000011504:DEBUG: void Multicast_Presence_Announcement() (1080):socket_sendto()
0000011505:DEBUG: void Multicast_Presence_Announcement() (1083):socket_close(multicast_socket)
0000011505:DEBUG: sock_result_t socket_close(sock_handle_t) (939):socket closed 20006138
0000011506:DEBUG: sock_result_t socket_send(sock_handle_t, const void*, socklen_t) (1002):Write 18 bytes to socket 536895672 result=0
0000011506:DEBUG: sock_result_t socket_send(sock_handle_t, const void*, socklen_t) (1002):Write 18 bytes to socket 536895672 result=0
0000011508:INFO : void handle_cloud_connection(bool) (282):Cloud connected
connected to the cloud!
0000011532:DEBUG: int read_packet_and_dispose(tcp_packet_t&, void*, int, wiced_tcp_socket_t*, int) (792):Socket 0 receive bytes 2 of 18
```

The source code is [here](https://github.com/rickkas7/photon-clouddebug/blob/master/clouddebug.cpp) so you can see how it works, but you kind of have to have a gcc-arm local development environment to get the system debug feature.

## Prerequisites 

- This only works with the Photon!
- You should have the [Particle CLI](https://docs.particle.io/guide/tools-and-features/cli/photon/) installed
- You must have a working dfu-util


## To Install

Because both debug system firmware and user firmware are required to get full debugging information, and downloading and installing all three pieces manually is a pain, I have a combined binary that contains all three parts in a single file.

> Technical note: This is actually system-part1, system-part2 (0.5.3) and the user firmware binary concatenated into a single file. It's not a monolithic binary, so you can actually flash new user firmware on top of it at 0x80A0000 and it will work properly.

Download the [combined.bin](https://github.com/rickkas7/photon-clouddebug/raw/master/combined.bin) file.

Put the Photon in DFU mode (blinking yellow) by pressing RESET and SETUP. Release RESET and continue to hold down SETUP while the LED blinks magenta until it blinks yellow, then release SETUP.

Issue the command:

```
dfu-util -d 2b04:d006 -a 0 -s 0x8020000:leave -D combined.bin
```

The Photon will restart. Immediately open a serial window. For example, using the CLI:

```
particle serial monitor
```


## To Remove

Using the Particle CLI, just update the system and user firmware binaries:

Put the Photon in DFU mode (blinking yellow) by pressing RESET and SETUP. Release RESET and continue to hold down SETUP while the LED blinks magenta until it blinks yellow, then release SETUP.

```
particle flash --usb tinker
```

Enter DFU (blinking yellow) mode again, then:

```
particle update
```




