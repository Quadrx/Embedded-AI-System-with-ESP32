# Embedded AI Thermal Prediction System

Distributed thermal monitoring and prediction system based on an ESP32-S3, a Raspberry Pi Zero 2 W, an MLX90614 infrared temperature sensor, Bluetooth Low Energy, TensorFlow Lite, and a custom PCB.

---

## Table of Contents

- [1. Project Overview](#1-project-overview)
- [2. Academic Scope and Development Stages](#2-academic-scope-and-development-stages)
- [3. Architecture Evolution](#3-architecture-evolution)
- [4. Repository Structure](#4-repository-structure)
- [5. Hardware Platform](#5-hardware-platform)
- [6. Standalone ESP32 Prototype](#6-standalone-esp32-prototype)
- [7. BLE Communication Prototype](#7-ble-communication-prototype)
- [8. Mini-Project 3](#8-mini-project-3)
- [9. Final Project Architecture](#9-final-project-architecture)
- [10. AI Model and Dataset](#10-ai-model-and-dataset)
- [11. End-to-End Operation](#11-end-to-end-operation)
- [12. Experimental Methodology](#12-experimental-methodology)
- [13. Experimental Evidence](#13-experimental-evidence)
- [14. Current Project Status](#14-current-project-status)
- [15. Hardware Issues and Recommendations](#15-hardware-issues-and-recommendations)
- [16. Limitations](#16-limitations)
- [17. How to Reproduce the Project](#17-how-to-reproduce-the-project)
- [18. Future Work](#18-future-work)
- [19. Conclusions](#19-conclusions)

---

## 1. Project Overview

This project presents the development of an intelligent thermal monitoring and prediction system based on an ESP32-S3, a Raspberry Pi Zero 2 W, and an MLX90614 infrared temperature sensor.

The system was developed progressively through several prototypes. The first implementation performed sensor acquisition, thermal processing, physical-model estimation, and TensorFlow Lite inference directly on the ESP32-S3. A later prototype validated wireless temperature transmission through Bluetooth Low Energy. The final architecture distributes the system tasks between the custom ESP32-S3 board and the Raspberry Pi Zero 2 W.

The main objective is to monitor the cooling behavior of an object and estimate the time required for the temperature difference between the object and its environment to reach 5 °C.

The complete target architecture is:

```text
MLX90614
    ↓
Custom ESP32-S3 PCB
    ↓ BLE temperature notifications
Raspberry Pi Zero 2 W
    ↓ preprocessing and TensorFlow Lite inference
Cooling-time prediction
    ↓ BLE prediction/control message
ESP32-S3
    ↓
Monitoring or actuator response
```

---

## 2. Academic Scope and Development Stages

The complete project was organized around two main academic stages: Mini-Project 3 and the Final Project.

### Mini-Project 3

Mini-Project 3 focused on the implementation of artificial intelligence, IoT communication, firmware, and software using the Raspberry Pi Zero 2 W.

During this stage, the thermal dataset was organized, the neural network was trained using TensorFlow/Keras, the model was converted to TensorFlow Lite, and BLE communication between the Raspberry Pi and the ESP32 was tested.

The objective was to verify that the Raspberry Pi could receive temperature information, preprocess the input variables, execute the trained model, and generate a numerical cooling-time prediction.

More information is available in:

[Mini-Project 3 Documentation](docs/miniproject-3.md)

### Final Project

The Final Project integrates the Raspberry Pi Zero 2 W with the custom ESP32-S3 PCB to create a complete IoT and AI solution.

In the target architecture, the ESP32-S3 acquires the MLX90614 measurements and publishes them through BLE. The Raspberry Pi receives the measurements, constructs the required temporal input vector, executes TensorFlow Lite inference, and sends the prediction back to the ESP32.

More information is available in:

[Final Project Documentation](docs/final-project.md)

---

## 3. Architecture Evolution

The repository contains implementations developed during different stages of the project. These implementations should not be interpreted as identical versions of the final system.

### Stage 1: Standalone ESP32 Implementation

The first firmware version placed most of the functionality directly on the ESP32-S3. This version included sensor acquisition, thermal filtering, physical-model estimation, temporal feature capture, and TensorFlow Lite inference.

This implementation demonstrated that the ESP32-S3 could perform the complete thermal processing sequence locally.

### Stage 2: BLE Communication Prototype

A separate BLE proof of concept was developed to validate wireless communication. In this implementation, the Raspberry Pi acted as the BLE server and the ESP32 acted as the BLE client.

The Raspberry Pi read the MLX90614 directly and transmitted messages such as:

```text
OBJ:26.15,AMB:24.80
```

The ESP32 connected to the server, subscribed to notifications, received the temperature values, and displayed them through the serial monitor.

This prototype validated BLE discovery, connection, notification, and real-time data transmission.

### Stage 3: Target Distributed Architecture

The target architecture reverses the BLE roles used in the initial prototype.

The ESP32-S3 becomes the GATT server and publishes the sensor measurements. The Raspberry Pi Zero 2 W becomes the GATT client and performs preprocessing, TensorFlow Lite inference, prediction generation, and higher-level decision-making.

This separation allows the ESP32-S3 to handle real-time hardware operations while the Raspberry Pi performs the more computationally demanding AI tasks.

---

## 4. Repository Structure

```text
.
├── README.md
├── ai/
│   ├── dataset_cooling.csv
│   └── train_model.py
├── ble/
│   ├── esp32/
│   │   └── ble_client.ino
│   └── raspberry/
│       └── mlx_ble.py
├── docs/
│   ├── ble-raspberry-esp32.md
│   ├── miniproject-3.md
│   ├── final-project.md
│   └── logs/
│       └── test-run-01.txt
├── firmware/
│   └── main.ino
└── hardware/
    ├── esp32(Final version).zip
    └── note.txt
```

The `firmware` folder contains the standalone ESP32-S3 implementation. The `ai` folder contains the thermal dataset and the neural-network training script. The `ble` folder contains the first BLE communication prototype. The `docs` folder contains the technical documentation for each project stage and the experimental serial logs. The `hardware` folder contains the compressed KiCad project for the custom PCB.

---

## 5. Hardware Platform

The project uses the following hardware components:

| Component | Function |
|---|---|
| ESP32-S3 N16R8 | Sensor acquisition, BLE communication, and embedded processing |
| Raspberry Pi Zero 2 W | AI inference, preprocessing, communication, and decision-making |
| MLX90614 | Infrared measurement of object and ambient temperature |
| Custom PCB | Integration of the ESP32-S3 and system connections |
| USB Type-C adapter | Programming and communication interface |

The MLX90614 provides two measurements:

```text
Tobj = object temperature
Tamb = ambient temperature
```

The thermal difference is calculated as:

```text
dT = Tobj - Tamb
```

The hardware design files are available in:

[KiCad Project Archive](hardware/esp32(Final%20version).zip)

---

## 6. Standalone ESP32 Prototype

The file `firmware/main.ino` contains the first complete embedded implementation.

This firmware initializes the MLX90614 sensor, continuously reads object and ambient temperature, calculates the thermal difference, filters the signal, detects the beginning of a cooling sequence, and captures the required thermal checkpoints.

The main captured values are:

| Variable | Meaning |
|---|---|
| `Tobj_0` | Initial object temperature |
| `Tamb_0` | Initial ambient temperature |
| `dT_0` | Initial temperature difference |
| `dT_5` | Temperature difference at 5 seconds |
| `dT_10` | Temperature difference at 10 seconds |
| `dT_20` | Temperature difference at 20 seconds |
| `dT_30` | Temperature difference at 30 seconds |

The firmware also implements a physical thermal model. A moving window of thermal-difference measurements is transformed using the natural logarithm and processed through linear regression to estimate the thermal time constant `tau`.

The resulting time constant is used to estimate the remaining time until the thermal difference reaches the 5 °C target.

The same firmware was prepared to execute a TensorFlow Lite model locally on the ESP32-S3. This version serves as proof that the complete sensing and prediction sequence can be executed directly on the microcontroller.

---

## 7. BLE Communication Prototype

The repository includes a BLE communication prototype with the following files:

```text
ble/raspberry/mlx_ble.py
ble/esp32/ble_client.ino
```

In this prototype, the Raspberry Pi acts as a BLE server. It reads object and ambient temperature from the MLX90614 through I2C and publishes the measurements using a BLE characteristic.

The ESP32 acts as a BLE client. It scans for a device named `raspberry_kevin`, connects to the corresponding service, subscribes to notifications, and prints each received message in the serial monitor.

The prototype uses the following configuration:

```text
Service UUID:        12345678-1234-5678-1234-56789abcdef0
Characteristic UUID: 12345678-1234-5678-1234-56789abcdef1
BLE device name:      raspberry_kevin
```

This implementation does not represent the final role distribution, but it demonstrates that communication between both platforms works correctly.

Detailed information is available in:

[BLE Communication Documentation](docs/ble-raspberry-esp32.md)

---

## 8. Mini-Project 3

Mini-Project 3 focused on implementing AI, IoT, firmware, and software around the Raspberry Pi Zero 2 W.

The neural network was trained beforehand using TensorFlow/Keras. The Raspberry Pi does not need to train the model during system operation. Instead, it loads the converted TensorFlow Lite model and performs inference using the previously learned weights.

The Raspberry-side inference process is expected to perform the following sequence:

1. Receive object and ambient temperature measurements.
2. Detect the beginning of a new cooling sequence.
3. Store the initial thermal conditions.
4. Capture the temperature difference at 5, 10, 20, and 30 seconds.
5. Construct the eight-variable input vector.
6. Normalize the input using the training parameters.
7. Execute the TensorFlow Lite interpreter.
8. Convert the normalized output back to seconds.
9. Send the prediction to the ESP32.

The TensorFlow Lite model uses an input tensor with shape `[1, 8]` and produces an output tensor with shape `[1, 1]`.

---

## 9. Final Project Architecture

The Final Project combines the Raspberry Pi Zero 2 W and the custom ESP32-S3 board into a distributed IoT and AI system.

### ESP32-S3 Responsibilities

The ESP32-S3 is responsible for reading the MLX90614 through I2C, calculating the instantaneous thermal difference, transmitting the measurements through BLE, receiving the prediction, and optionally applying a control decision.

The final architecture uses one BLE service and two characteristics:

| BLE Element | Function | Direction |
|---|---|---|
| Main service | Groups the thermal prediction functions | — |
| Temperature characteristic | Publishes `Tobj,Tamb` | ESP32 → Raspberry Pi |
| Prediction/control characteristic | Receives prediction or commands | Raspberry Pi → ESP32 |

A temperature message may use a simple format such as:

```text
36.42,24.87
```

### Raspberry Pi Responsibilities

The Raspberry Pi Zero 2 W scans for the ESP32 device, establishes the BLE connection, subscribes to temperature notifications, constructs the 0–30 second temporal window, executes TensorFlow Lite inference, and sends the resulting prediction to the ESP32.

### Distributed Processing

The ESP32-S3 performs time-sensitive hardware operations, while the Raspberry Pi performs preprocessing, AI inference, and high-level decision-making.

This architecture prepares the system for future actuator integration.

---

## 10. AI Model and Dataset

The dataset is stored in:

[Cooling Dataset](ai/dataset_cooling.csv)

Each row represents one complete cooling experiment. The model receives eight variables and predicts one numerical output.

| Variable | Description |
|---|---|
| `momento_dia` | Time-of-day category |
| `Tobj_0` | Initial object temperature |
| `Tamb_0` | Initial ambient temperature |
| `dT_0` | Initial thermal difference |
| `dT_5` | Thermal difference at 5 seconds |
| `dT_10` | Thermal difference at 10 seconds |
| `dT_20` | Thermal difference at 20 seconds |
| `dT_30` | Thermal difference at 30 seconds |
| `tiempo_hasta_dt5` | Time required to reach `dT = 5 °C` |

The model learns a function of the form:

```text
f(
  momento_dia,
  Tobj_0,
  Tamb_0,
  dT_0,
  dT_5,
  dT_10,
  dT_20,
  dT_30
) = tiempo_hasta_dt5
```

### Neural-Network Architecture

```text
Input(8)
   ↓
Dense(16, ReLU)
   ↓
Dense(8, ReLU)
   ↓
Dense(1, Linear)
```

The model uses the Adam optimizer, mean squared error as its loss function, and mean absolute error as an evaluation metric.

Input normalization is performed using:

```text
X_norm = (X - X_mean) / X_std
```

The model output is converted back to seconds using:

```text
prediction_seconds = prediction_norm × y_std + y_mean
```

The normalization parameters used during inference must be identical to the parameters calculated during training.

---

## 11. End-to-End Operation

The final distributed system is expected to operate as follows:

1. The MLX90614 measures object and ambient temperature.
2. The ESP32-S3 reads the sensor through I2C.
3. The ESP32 calculates `dT = Tobj - Tamb`.
4. The ESP32 publishes the measurements through BLE.
5. The Raspberry Pi receives approximately one sample per second.
6. The Raspberry detects the beginning of the cooling sequence.
7. The variables from 0 to 30 seconds are captured.
8. The eight model inputs are normalized.
9. TensorFlow Lite executes the neural network.
10. The normalized prediction is converted back to seconds.
11. The Raspberry sends the prediction to the ESP32.
12. The ESP32 displays the prediction or applies a control action.

```text
Sensor
  → ESP32-S3
  → BLE
  → Raspberry Pi Zero 2 W
  → TensorFlow Lite
  → Prediction
  → BLE
  → ESP32-S3
  → Monitoring or actuator
```

---

## 12. Experimental Methodology

The thermal experiments were performed indoors under relatively closed environmental conditions. Measurements were collected at different times of the day to represent morning, afternoon, and night conditions.

Ceramic surfaces were selected because their temperature changed more slowly than other tested materials. This provided enough time to observe the cooling process and capture the required temporal variables.

The MLX90614 sensor was positioned above the object and aimed toward its center.

During some experiments, the surrounding area heated more than expected. This caused the ambient-temperature measurement to increase significantly and reduced the consistency of the thermal data.

A lighter was later used as a more controllable heat source for demonstration purposes because it produced lower and more manageable temperatures than the stove-based setup.

---

## 13. Experimental Evidence

The repository includes the dataset, firmware, BLE prototype, serial logs, hardware files, and a demonstration video.

The serial log from a complete test run is available at:

[Test Run Log](docs/logs/test-run-01.txt)

The hardware design is available at:

[KiCad Project Archive](hardware/esp32(Final%20version).zip)

The project demonstration video is available at:

[Watch the Demo Video](https://drive.google.com/file/d/1Ku_SDlaufHtSQT0y7ulRh_jtI_WYhIBA/view?usp=sharing)

---

## 14. Current Project Status

| Component | Status | Result |
|---|---|---|
| MLX90614 sensor acquisition | Completed | Object and ambient temperatures acquired |
| Standalone ESP32 firmware | Completed | Thermal processing and prediction prototype |
| Custom ESP32-S3 PCB | Manufactured and tested | Functional with identified hardware issues |
| Thermal dataset | Available | Training variables and target defined |
| TensorFlow/Keras training | Completed | Regression model trained |
| TensorFlow Lite conversion | Completed | Model prepared for deployment |
| Initial BLE prototype | Completed | Raspberry-to-ESP32 notifications verified |
| TensorFlow Lite loading on Raspberry Pi | Verified | Input and output tensors recognized |
| Final bidirectional BLE architecture | In development | ESP32 server and Raspberry client must be fully integrated |
| Prediction returned to ESP32 | In development | Control characteristic must be completed |
| Actuator response | Planned | Final control action must be defined |

---

## 15. Hardware Issues and Recommendations

### USB Type-C Adapter

A female header was originally used to make the USB Type-C adapter removable. This decision was intended to simplify replacement if the PCB failed. However, the removable connection introduced electrical and mechanical problems.

For future PCB revisions, the Type-C adapter should be soldered directly to the board.

### ESP32-S3 Antenna Area

The antenna area of the ESP32-S3 N16R8 module did not receive sufficient mechanical support in the final PCB design.

Future versions should improve the board structure around the antenna region while respecting the antenna keep-out requirements.

### Grounding

Additional holes were created to establish ground connections between PCB layers. Male-to-male headers were used as a practical solution for connecting the upper layer to ground.

Future revisions should use properly designed plated vias and ground stitching instead of external header connections.

---

## 16. Limitations

The experimental dataset was collected in a limited indoor environment. Airflow, environmental variation, sensor distance, heat-source consistency, and surface characteristics were not fully controlled.

The increase in ambient temperature during some experiments affected the measured thermal difference. As a result, some AI predictions were not fully consistent with the expected physical cooling behavior.

The initial BLE prototype also uses different client and server roles from the target final architecture. Although it validates communication, additional development is required to complete the final bidirectional system.

---

## 17. How to Reproduce the Project

### AI Training

Install the required Python packages:

```bash
pip install numpy pandas tensorflow matplotlib
```

Place `dataset_cooling.csv` in the same working directory as `train_model.py`, then execute:

```bash
python train_model.py
```

The script loads the dataset, normalizes the variables, trains the neural network, evaluates the predictions, and prepares the model for TensorFlow Lite conversion.

### Standalone ESP32 Firmware

Open `firmware/main.ino` in Arduino IDE. Install the ESP32 board support, the Adafruit MLX90614 library, and the required TensorFlow Lite Micro library.

Select the correct ESP32-S3 board and serial port, upload the firmware, and open the serial monitor at 115200 baud.

### BLE Prototype

Run the Raspberry Pi BLE server:

```bash
python3 mlx_ble.py
```

Upload `ble_client.ino` to the ESP32 and open the serial monitor. The ESP32 should detect `raspberry_kevin`, connect to the BLE service, subscribe to notifications, and print the received temperature values.

### Final Architecture

The final architecture requires an ESP32 GATT server that publishes temperature measurements and a Raspberry Pi GATT client that performs TensorFlow Lite inference and writes the prediction back to the ESP32.

These final software components are still part of the ongoing integration stage.

---

## 18. Future Work

Future development should focus on completing the final bidirectional BLE architecture, integrating the temporal feature capture with Raspberry Pi inference, sending the prediction back to the ESP32, and defining an actuator response.

The PCB should also be revised to improve the USB Type-C connection, antenna support, grounding, and layer interconnection.

Additional thermal experiments should be performed under better-controlled conditions. The dataset should include more materials, environmental conditions, starting temperatures, and repeated measurements.

The AI model should then be retrained and evaluated using quantitative error metrics and comparisons against the physical thermal model.

---

## 19. Conclusions

This project demonstrates the progressive development of an embedded AI and IoT thermal prediction system.

The standalone ESP32 prototype validated sensor acquisition, thermal processing, physical-model estimation, and embedded inference. The first BLE prototype validated real-time wireless communication between the Raspberry Pi and the ESP32. Mini-Project 3 verified the AI and IoT processing workflow on the Raspberry Pi Zero 2 W.

The Final Project extends these results into a distributed architecture in which the custom ESP32-S3 PCB performs real-time sensing and BLE communication while the Raspberry Pi executes TensorFlow Lite inference and decision-making.

Although the final bidirectional integration is still under development, the project already demonstrates the essential components required for a complete intelligent thermal monitoring and control system.
