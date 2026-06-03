# BLE Communication Between Raspberry Pi and ESP32

## Overview

This document describes an additional communication module developed as part of the project. In this module, a Raspberry Pi connected to an MLX90614 infrared sensor works as a Bluetooth Low Energy server, while an ESP32 works as a BLE client. The purpose of this stage is to read temperature data on the Raspberry Pi side and transmit it wirelessly to the ESP32 in real time.

The Raspberry Pi is responsible for data acquisition. It reads both ambient temperature and object temperature through the I2C bus and publishes the measured values through a BLE characteristic. The ESP32 discovers the Raspberry Pi server, connects to it, subscribes to notifications, and displays the received information in the serial monitor.

## Objective

The objective of this module is to establish a simple and functional BLE communication channel between a Raspberry Pi and an ESP32 for real-time transmission of thermal measurements obtained from the MLX90614 sensor.

## File Structure

```text
ble/
├── esp32/
│   └── ble_client.ino
└── raspberry/
    └── mlx_ble.py
```

The file `ble/raspberry/mlx_ble.py` contains the BLE server implementation for the Raspberry Pi. The file `ble/esp32/ble_client.ino` contains the BLE client implementation for the ESP32.

## Raspberry Pi BLE Server

The Raspberry Pi script is written in Python and uses the MLX90614 sensor through I2C together with the `bless` library to expose a BLE service.

The program starts by initializing the I2C bus and the MLX90614 sensor. After that, it creates a BLE server named `raspberry_kevin`. A service is registered using the UUID `12345678-1234-5678-1234-56789abcdef0`, and a characteristic is created using the UUID `12345678-1234-5678-1234-56789abcdef1`.

This characteristic is configured with read and notify properties. Once the BLE server starts, the Raspberry Pi enters an infinite loop where it reads the ambient temperature and the object temperature from the sensor, formats the values as text, updates the BLE characteristic, and sends the value to subscribed clients.

The transmitted data follows this format:

```text
OBJ:26.15,AMB:24.80
```

In this message, `OBJ` represents object temperature and `AMB` represents ambient temperature.

## ESP32 BLE Client

The ESP32 code is written in Arduino C++ and acts as a BLE client. The program scans nearby BLE devices and looks for one with the advertised name `raspberry_kevin`. When the correct device is found, the ESP32 stops scanning and attempts to connect.

After a successful connection, the ESP32 searches for the BLE service using the expected UUID and then locates the target characteristic. If the characteristic supports notifications, the ESP32 subscribes to them and waits for updates.

Every time a new BLE notification is received, the callback function processes the incoming bytes and prints the received text to the serial monitor. In this way, the ESP32 continuously displays the temperatures transmitted by the Raspberry Pi.

## Communication Flow

The complete communication process works as follows.

First, the MLX90614 sensor measures the ambient and object temperature. Then the Raspberry Pi reads these values through I2C. After acquiring the measurements, the Raspberry Pi formats the data as a text string and updates the BLE characteristic. The BLE server sends this value as a notification. The ESP32, acting as a client, receives the notification, decodes the text, and prints the result in the serial monitor.

This creates a complete real-time data path from the thermal sensor to the wireless receiver.

## UUID Configuration

The BLE communication uses the following identifiers:

```text
Service UUID:        12345678-1234-5678-1234-56789abcdef0
Characteristic UUID: 12345678-1234-5678-1234-56789abcdef1
Device Name:         raspberry_kevin
```

These values must match on both the Raspberry Pi server and the ESP32 client for the connection to work properly.

## Software and Technologies

This module combines Python on Raspberry Pi and Arduino C++ on ESP32. The Raspberry Pi side uses Python together with the `bless` BLE library, while the ESP32 side uses the Arduino BLE libraries. The sensor communication is performed through I2C, and the wireless communication is performed through Bluetooth Low Energy.

The main technologies used in this module are Raspberry Pi, ESP32, MLX90614, Python, Arduino IDE, BLE, and I2C.

## Practical Use

This BLE module is useful when sensing and embedded visualization are distributed across different devices. It can also serve as a communication layer for future versions of the project in which the sensing stage is separated from the processing or display stage.

Because the transmitted message uses a human-readable format, the module is also convenient for debugging and demonstration purposes.

## Expected Result

When the system operates correctly, the Raspberry Pi prints the transmitted data continuously, and the ESP32 shows the same information in the serial monitor after receiving BLE notifications. The result is a working wireless temperature monitoring link between both devices.

## Conclusion

This BLE module demonstrates a practical communication layer between a Raspberry Pi and an ESP32. The Raspberry Pi successfully acquires temperature data from the MLX90614 sensor and publishes it through BLE, while the ESP32 successfully receives and displays the transmitted values. This makes the module a useful extension of the main thermal monitoring project and a solid basis for future distributed embedded systems.
