# Initial BLE Prototype: Raspberry Pi Server and ESP32 Client

> **Prototype note:** This document describes the first BLE proof of concept. In this version, the Raspberry Pi acts as the BLE server and reads the MLX90614 directly, while the ESP32 acts as the BLE client. The Final Project reverses these roles: the ESP32-S3 becomes the BLE server and the Raspberry Pi Zero 2 W becomes the BLE client and AI-processing unit.

## 1. Purpose

This prototype was created to verify that temperature measurements could be transmitted in real time between a Raspberry Pi and an ESP32 through Bluetooth Low Energy.

The communication path is:

```text
MLX90614 → Raspberry Pi → BLE notifications → ESP32 → Serial Monitor
```

The related files are:

```text
ble/raspberry/mlx_ble.py
ble/esp32/ble_client.ino
```

---

## 2. BLE Configuration

Both programs must use the same service and characteristic identifiers:

```text
Service UUID:        12345678-1234-5678-1234-56789abcdef0
Characteristic UUID: 12345678-1234-5678-1234-56789abcdef1
Device name:         raspberry_kevin
```

The service groups the thermal communication functionality. The characteristic stores the latest text message and supports read and notify operations.

---

## 3. Raspberry Pi Code: `mlx_ble.py`

### Imported modules

```python
import asyncio
import board
import busio
import adafruit_mlx90614
```

`asyncio` manages the asynchronous BLE server loop. `board` and `busio` expose the Raspberry Pi I2C pins. `adafruit_mlx90614` communicates with the sensor.

The Bless imports provide the GATT server, characteristic properties, and permissions.

### I2C initialization

```python
i2c = busio.I2C(board.SCL, board.SDA)
mlx = adafruit_mlx90614.MLX90614(i2c)
```

The Raspberry Pi opens its I2C bus and creates the sensor object. If wiring, permissions, or the I2C configuration are incorrect, the script fails before starting BLE.

### BLE server creation

```python
server = BlessServer(name="raspberry_kevin")
```

The advertised name is important because the ESP32 searches for this exact text.

The code adds the service and a characteristic with read and notify properties. The initial value is `Inicio`, which confirms that the characteristic exists before the first sensor sample.

### Server start

```python
await server.start()
```

After this call, the Raspberry Pi begins advertising the service. The ESP32 can then discover and connect to it.

### Sensor loop

The code continuously reads:

```python
amb = mlx.ambient_temperature
obj = mlx.object_temperature
```

It formats the values as:

```text
OBJ:26.15,AMB:24.80
```

This format is human-readable and easy to debug, although it requires text parsing on the receiver.

### Characteristic update

The text is encoded to bytes and assigned to the characteristic. `server.update_value()` sends a notification to subscribed clients.

The loop waits one second before the next update, producing approximately one message per second.

---

## 4. ESP32 Code: `ble_client.ino`

### BLE initialization

The ESP32 includes the BLE device, scan, and client classes. It defines the same UUIDs used by the Raspberry Pi.

### Notification callback

```cpp
static void notifyCallback(...)
```

BLE notifications arrive as a byte array. The callback iterates over the bytes, converts each one to a character, and prints the complete text in the serial monitor.

The prototype does not separate `OBJ` and `AMB` into floating-point variables. It only verifies that the message is received correctly.

### Scan callback

`MyAdvertisedDeviceCallbacks::onResult()` is called for every discovered BLE device. The code prints the device name and compares it against `raspberry_kevin`.

When the expected server is found, the ESP32 stores the device information, stops scanning, and sets `doConnect = true`.

### Connection function

`connectToServer()` creates a BLE client and connects to the discovered device. It then locates the service and characteristic using the configured UUIDs.

Each operation is checked. If the service or characteristic is not found, the client disconnects and reports the failure.

If the characteristic supports notifications, the client registers `notifyCallback` and becomes subscribed.

### Main loop and reconnection

The main loop attempts the connection after discovery. Once connected, it periodically checks `pClient->isConnected()`.

If the link is lost, the code marks the connection as inactive and restarts the scan. This gives the prototype a simple recovery mechanism.

---

## 5. Complete Prototype Flow

```text
1. MLX90614 measures object and ambient temperature.
2. Raspberry Pi reads both values through I2C.
3. Raspberry Pi formats OBJ and AMB as text.
4. Bless updates the BLE characteristic.
5. ESP32 receives a notification.
6. ESP32 prints the text in the serial monitor.
7. The sequence repeats every second.
```

---

## 6. Installation

The Raspberry Pi packages can be installed manually:

```bash
pip install bless adafruit-blinka adafruit-circuitpython-mlx90614
```

I2C must be enabled in the Raspberry Pi configuration. The script can then be executed with:

```bash
python3 ble/raspberry/mlx_ble.py
```

The ESP32 code is uploaded through Arduino IDE. The serial monitor should be opened at 115200 baud.

---

## 7. Result

This implementation verifies the BLE communication layer. It proves that a Raspberry Pi can advertise sensor data and that an ESP32 can discover the device, connect, subscribe, receive notifications, and recover from a lost link.

It is retained as development evidence, but it should not be confused with the final architecture.

