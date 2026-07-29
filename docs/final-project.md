# Final Project: Distributed ESP32-S3 and Raspberry Pi Thermal Prediction System

## Overview

The Final Project integrates a custom ESP32-S3 board, a Raspberry Pi Zero 2 W, an MLX90614 infrared temperature sensor, Bluetooth Low Energy, and TensorFlow Lite into a distributed intelligent thermal prediction system.

The system is designed to observe the cooling behavior of an object and estimate the time required for the temperature difference between the object and the environment to reach 5 °C.

The final architecture separates real-time hardware functions from AI processing:

| Device | Main responsibility |
|---|---|
| ESP32-S3 | Sensor acquisition, BLE server, and actuator interface |
| Raspberry Pi Zero 2 W | BLE client, preprocessing, inference, and decision-making |
| MLX90614 | Object and ambient temperature measurement |
| TensorFlow Lite model | Cooling-time prediction |
| BLE | Bidirectional wireless communication |

---

## Final Objective

The objective is to combine the custom ESP32-S3 PCB with the Raspberry Pi Zero 2 W to create a complete IoT and AI solution.

The ESP32-S3 acquires the thermal measurements and transmits them wirelessly. The Raspberry Pi receives the data, constructs the neural-network inputs, executes the TensorFlow Lite model, and returns the predicted cooling time.

The ESP32 can then display the result or use it as an input for a future control action.

---

## Final System Architecture

The final data path is:

```text
MLX90614
    ↓
ESP32-S3
    ↓ BLE temperature notification
Raspberry Pi Zero 2 W
    ↓
Input capture and normalization
    ↓
TensorFlow Lite inference
    ↓
Cooling-time prediction
    ↓ BLE prediction/control message
ESP32-S3
    ↓
Monitoring or actuator
```

This architecture distributes the work between the two processing devices.

The ESP32-S3 handles time-sensitive acquisition and hardware interaction. The Raspberry Pi performs the more computationally demanding tasks associated with preprocessing, AI inference, and decision-making.

---

## ESP32-S3 Responsibilities

The ESP32-S3 is responsible for interacting directly with the MLX90614 sensor and exposing the data through Bluetooth Low Energy.

Its main functions are:

1. Initialize the I2C interface.
2. Read the object temperature.
3. Read the ambient temperature.
4. Calculate the instantaneous temperature difference.
5. Publish the temperature data through a BLE characteristic.
6. Receive the prediction through a second BLE characteristic.
7. Display the result or use it for future actuator control.

The temperature difference is calculated as:

```text
dT = Tobj - Tamb
```

---

## MLX90614 Connection

The project presentation defines the following I2C pin configuration:

| Signal | ESP32-S3 pin |
|---|---:|
| SDA | GPIO 47 |
| SCL | GPIO 48 |

The ESP32 obtains approximately one new temperature measurement per second.

The transmitted packet contains the object temperature and ambient temperature separated by a comma:

```text
36.42,24.87
```

The first value corresponds to `Tobj`, and the second value corresponds to `Tamb`.

---

## Raspberry Pi Responsibilities

The Raspberry Pi Zero 2 W acts as the BLE client and AI processing unit.

Its responsibilities are:

1. Scan for the ESP32 BLE device.
2. Connect to the main thermal prediction service.
3. Subscribe to temperature notifications.
4. Receive approximately one `Tobj,Tamb` pair per second.
5. Convert the received text into floating-point values.
6. Detect the beginning of a cooling sequence.
7. Capture the 0–30 second temporal window.
8. Normalize the eight input variables.
9. Execute TensorFlow Lite inference.
10. Convert the normalized prediction back to seconds.
11. Send the prediction to the ESP32.
12. Support future high-level control decisions.

---

## BLE Architecture

The ESP32-S3 works as a GATT server, while the Raspberry Pi Zero 2 W works as a GATT client.

| Device | BLE role | Operation |
|---|---|---|
| ESP32-S3 | GATT server | Publishes characteristics and sends notifications |
| Raspberry Pi Zero 2 W | GATT client | Connects, subscribes, receives data, and writes predictions |

The target ESP32 device name is:

```text
ESP32_SENSOR
```

The BLE connection uses one main service and two characteristics.

| BLE element | Identifier | Function | Direction |
|---|---|---|---|
| Main service | `...abcdef0` | Groups the system characteristics | — |
| Temperature characteristic | `...abcdef1` | Sends `Tobj,Tamb` | ESP32 → Raspberry Pi |
| Prediction/control characteristic | `...abcdef2` | Receives prediction or command | Raspberry Pi → ESP32 |

The complete UUID values should remain identical in the ESP32 and Raspberry Pi programs.

---

## BLE Communication Sequence

The Raspberry Pi follows this connection process:

1. Starts scanning nearby BLE devices.
2. Searches for `ESP32_SENSOR`.
3. Connects to the configured service.
4. Locates the temperature characteristic.
5. Locates the prediction/control characteristic.
6. Subscribes to temperature notifications.
7. Receives a new temperature pair approximately every second.
8. Executes the prediction after collecting the required data.
9. Writes the result to the prediction/control characteristic.

This creates bidirectional communication:

```text
ESP32 → temperature measurements → Raspberry Pi
ESP32 ← prediction or control value ← Raspberry Pi
```

---

## AI Model Input Construction

The Raspberry Pi must build the following eight-value input vector:

```text
[
  momento_dia,
  Tobj_0,
  Tamb_0,
  dT_0,
  dT_5,
  dT_10,
  dT_20,
  dT_30
]
```

The temporal capture process begins when the system detects a new cooling event.

At the initial instant, the Raspberry stores:

```text
Tobj_0
Tamb_0
dT_0
```

It then waits and records:

```text
dT_5
dT_10
dT_20
dT_30
```

Once the complete vector is available, the Raspberry normalizes the data and executes the TensorFlow Lite model.

---

## TensorFlow Lite Inference

The neural network was trained previously using TensorFlow/Keras and converted to TensorFlow Lite.

The Raspberry Pi does not modify or retrain the model during operation. It only performs inference.

The model architecture is:

```text
Input(8)
   ↓
Dense(16, ReLU)
   ↓
Dense(8, ReLU)
   ↓
Dense(1, Linear)
```

The input normalization is:

```text
X_norm = (X - X_mean) / X_std
```

The output is converted back to seconds using:

```text
prediction_seconds = prediction_norm × y_std + y_mean
```

The TensorFlow Lite interpreter recognizes:

```text
Input:  [1, 8]
Output: [1, 1]
```

---

## End-to-End Operation

The complete system operation is:

| Step | Process |
|---:|---|
| 1 | The MLX90614 measures `Tobj` and `Tamb` |
| 2 | The ESP32-S3 reads the sensor through I2C |
| 3 | The ESP32 publishes the measurements through BLE |
| 4 | The Raspberry Pi receives the samples |
| 5 | The Raspberry constructs the 0–30 second temporal window |
| 6 | The eight model inputs are normalized |
| 7 | TensorFlow Lite generates the prediction |
| 8 | The prediction is converted back to seconds |
| 9 | The Raspberry sends the result to the ESP32 |
| 10 | The ESP32 displays the result or applies a control action |

---

## Custom PCB Integration

The Final Project uses the custom PCB developed for the ESP32-S3 system.

The PCB project is available in:

[KiCad Project Archive](../hardware/esp32(Final%20version).zip)

The board integrates the ESP32-S3 and the connections required for sensor acquisition, power, programming, grounding, and future control interfaces.

Several practical issues were identified during assembly.

The removable USB Type-C adapter created connection problems. A future revision should solder the adapter directly to the PCB.

The ESP32-S3 antenna area also requires improved mechanical and layout treatment. The design should respect the antenna keep-out region while providing appropriate board support.

Ground connections between layers were implemented using additional holes and external headers. Future versions should use properly designed plated vias and ground stitching.

---

## Current Project Status

| Component | Status | Result |
|---|---|---|
| MLX90614 and ESP32 acquisition | Working | Temperature readings obtained through I2C |
| BLE communication | Working | Notifications and characteristic writes verified |
| Thermal dataset | Available | Temporal variables and target defined |
| TensorFlow/Keras training | Completed | Regression model trained |
| TensorFlow Lite conversion | Completed | Model prepared for Raspberry Pi execution |
| Raspberry Pi inference | Verified | Model loads and tensor shapes are correct |
| Custom ESP32-S3 PCB | Manufactured | Tested with identified hardware issues |
| Final integration | In development | Acquisition, temporal window, and inference must be combined |
| Prediction return to ESP32 | Part of final integration | Control characteristic must be completed |
| Actuator response | Planned | Final control logic must be defined |

The verified components demonstrate that the work can be distributed between the ESP32 and Raspberry Pi. However, the full continuous application still requires final integration.

---

## Relationship with the BLE Prototype

The repository also contains an earlier BLE prototype in which the Raspberry Pi acts as the BLE server and the ESP32 acts as the client.

That prototype was used to verify BLE discovery, connection, subscription, and temperature transmission.

The Final Project uses the opposite role distribution:

```text
Initial BLE prototype:
Raspberry Pi = server
ESP32 = client

Final architecture:
ESP32-S3 = server
Raspberry Pi Zero 2 W = client
```

The initial prototype should therefore be interpreted as communication validation rather than the final implementation.

More information is available in:

[Initial BLE Prototype Documentation](ble-raspberry-esp32.md)

---

## Expected Final Result

The expected final system should operate continuously without manual intervention.

The ESP32-S3 should publish thermal measurements, the Raspberry Pi should collect the required time window and calculate the prediction, and the ESP32 should receive the estimated remaining time.

The resulting prediction can later be used to:

- display a cooling-time estimate;
- activate a visual or audible notification;
- control an actuator;
- stop or modify a thermal process;
- report the system state to another IoT platform.

---

## Pending Tasks

The remaining integration work includes combining BLE acquisition with temporal feature capture, executing inference automatically after 30 seconds, writing the prediction to the ESP32 control characteristic, and validating the complete cycle using real measurements.

The prediction error should also be quantified by comparing the TensorFlow Lite output with the real measured time required to reach `dT = 5 °C`.

---

## Conclusions

The Final Project proposes a distributed architecture that combines the real-time acquisition capability of the ESP32-S3 with the processing capability of the Raspberry Pi Zero 2 W.

Bluetooth Low Energy provides bidirectional wireless communication between both platforms. TensorFlow Lite allows the trained neural network to run on the Raspberry Pi without retraining.

The project has already verified sensor acquisition, BLE communication, dataset preparation, model training, TensorFlow Lite conversion, and model loading on the Raspberry Pi.

The final integration remains under development. The next stage is to validate continuous real-time prediction, send the result back to the ESP32, and quantify the prediction error against experimental measurements.
