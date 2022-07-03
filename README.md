# Create a multiprotocol network (Thread, Zigbee, BLE)
## Description
This project demonstrates is about building an multiprotocol IoT (Thread + Zigbee + Bluetooth-Low-Energy) network

![topology](https://github.com/thong-phn/multiprotocol_thesis/blob/main/images/topology.png)
![gateway](https://github.com/thong-phn/multiprotocol_thesis/blob/main/images/gateway.png)

## Hardware 
Multiprotocol Gateway:
- Linux host: Raspberry PI 3/Any PC runs Linux
- Thread-NCP: nRF52840 Dongle
- Zigbee-NCP: EFR32MG12

End devices:
- Thread nodes: nRF52840
- Zigbee nodes: EFR32MG12

Android phone:
- Support BLE
- Android version 11+

## SDK and Toolchain
Multiprotocol Gateway:
- Ubuntu 18.04 (Ubuntu 20.04 has some problems with lib packages)

Developing Thread End devices: 
- SDK: nRF SDK for Thread and Zigbee v4
- IDE: SEGGER Embedded Studio
- Toolchain: GNU GCC 10.3

Developing Zigbee End devices:
- SDK: EmberZNet v2.2
- IDE: Simplicity Studio 4
- Toolchain: IAR Workbench 

## How to test
### Flash firmware to end device
1. Install the right environment for building the end devices' firmware:
    1. Nordic (Thread + BLE): https://infocenter.nordicsemi.com/topic/struct_sdk/struct/sdk_thread_zigbee_latest.html
    2. Silabs (Zigbee + BLE): https://www.silabs.com/documents/public/application-notes/AN0822-simplicity-studio-user-guide.pdf
2. Modify the GPIO, if needed.
3. Build and flash the firmware.
4. Open serial port to view log.

### Install the gateway
1. Plug in 2 NCPs
2. Launch the Thread Gateway by running the docker file
```
docker run --name otbr-mqtt-sn -p 8080:80 --dns=127.0.0.1 -it --privileged otbr-mqtt-sn
```
3. Launch the Zigbee Gateway 
```
cd Z3GatewayHost/build/exe
./Z3GatewayHost -n 0 -p /dev/ttyACM0
```
## Network information

| Parameter            | Command line option             | Default value                    | 
|----------------------|:--------------------------------|:---------------------------------|
| Network Name         |  --network-name                 | OTBR-MQTT-SN                     |
| RCP Serial Port      |  --radio-url                    | spinel+hdlc+uart:///dev/ttyACM0  |
| PAN ID               |  --panid                        | 0xABCD                           |
| Extended PAN ID      |  --xpanid                       | DEAD00BEEF00CAFE                 |
| Channel              |  --channel                      | 11                               |
| Network Key          |  --network-key                  | 00112233445566778899AABBCCDDEEFF |
| Network PSKc         |  --pskc                         | 5ce66d049d007088ad900dfcc2a55ee3 |
| TUN Interface Name   |  --interface                    | wpan0                            |
| NAT64 Prefix         |  --nat64-prefix                 | 64:ff9b::/96                     |
| Default prefix route |  --disable-default-prefix-route | Enabled                          |
| Default prefix slaac |  --disable-default-prefix-slaac | Enabled                          |
| Backbone Interface   |  --backbone-interface           | eth0                             |
| MQTT Broker          |  --broker                       | mqtt.eclipseprojects.io          |

## Reference
1. Zigbee Gateway: https://www.mouser.com/datasheet/2/368/UG129-775963.pdf


