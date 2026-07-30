# Embedded AI Thermal Prediction System

Distributed thermal monitoring and prediction project based on an ESP32-S3, a Raspberry Pi Zero 2 W, an MLX90614 infrared temperature sensor, Bluetooth Low Energy, TensorFlow/Keras, TensorFlow Lite, and a custom PCB.

---

## Table of Contents

- [1. Project Overview](#1-project-overview)
- [2. Development Stages](#2-development-stages)
- [3. Repository Structure](#3-repository-structure)
- [4. Hardware Platform](#4-hardware-platform)
- [5. Standalone ESP32 Prototype](#5-standalone-esp32-prototype)
- [6. AI Training](#6-ai-training)
- [7. Initial BLE Prototype](#7-initial-ble-prototype)
- [8. Mini-Project 3](#8-mini-project-3)
- [9. Final Project Architecture](#9-final-project-architecture)
- [10. Dataset and Model Variables](#10-dataset-and-model-variables)
- [11. Experimental Methodology](#11-experimental-methodology)
- [12. Experimental Evidence](#12-experimental-evidence)
- [13. Current Status](#13-current-status)
- [14. Hardware Issues and Recommendations](#14-hardware-issues-and-recommendations)
- [15. Limitations](#15-limitations)
- [16. Reproduction Guide](#16-reproduction-guide)
- [17. Future Work](#17-future-work)
- [18. Conclusions](#18-conclusions)

---

## 1. Project Overview

This repository documents the progressive development of an intelligent thermal monitoring system. The system measures the temperature of an object and its environment, follows the cooling process, and estimates the time required for the temperature difference to reach 5 °C.

The project evolved through three implementations. The first version executed sensing, filtering, physical thermal estimation, and AI inference directly on the ESP32-S3. A second version validated BLE communication with the Raspberry Pi acting as server and the ESP32 acting as client. The final architecture reverses those BLE roles: the ESP32-S3 publishes the temperature measurements, and the Raspberry Pi Zero 2 W receives the data, builds the temporal input vector, executes TensorFlow Lite inference, and returns the prediction.

The target data flow is:

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

## 2. Development Stages

### Standalone ESP32 implementation

The first implementation is stored in `firmware/main.ino`. It reads the MLX90614, filters the thermal difference, captures the required temporal samples, estimates the thermal time constant `tau`, and was prepared to execute a TensorFlow Lite Micro model on the ESP32-S3.

### Initial BLE proof of concept

The first BLE test is stored under `ble/esp32/` and `ble/raspberry/`. In this version, the Raspberry Pi reads the MLX90614 and acts as the BLE server, while the ESP32 acts as the client. This stage validates device discovery, connection, notifications, and real-time temperature transmission.

### Final distributed system

The final Arduino code makes the ESP32-S3 a BLE server named `ESP32_SENSOR`. The final Raspberry Pi code acts as the BLE client, receives the measurements, captures the values at 0, 5, 10, 20, and 30 seconds, normalizes the eight model inputs, executes inference, and writes the predicted remaining time back to the ESP32.

Detailed documents are available here:

- [Mini-Project 3](docs/miniproject-3.md)
- [Initial BLE Prototype](docs/ble-raspberry-esp32.md)
- [Final Project](docs/final-project.md)

---

## 3. Repository Structure

The following structure matches the current repository and the new final-project code files. No `requirements.txt`, migration guide, or committed `.tflite` file is assumed.

```text
.
├── README.md
├── ai/
│   ├── dataset_cooling.csv
│   ├── dataset_enfriamiento.csv
│   └── train_model.py
├── ble/
│   ├── esp32/
│   │   ├── ble_client.ino
│   │   └── esp32_ble_server_final.ino
│   └── raspberry/
│       ├── mlx_ble.py
│       └── raspberry_ai_client_final.py
├── docs/
│   ├── ble-raspberry-esp32.md
│   ├── final-project.md
│   ├── miniproject-3.md
│   ├── images/
│   │   ├── PCB design.jpeg
│   │   └── Prototipo.jpeg
│   ├── logs/
│   │   └── test-run-01.txt
│   └── videos/
│       └── Diseño final.mp4
├── firmware/
│   └── main.ino
└── hardware/
    ├── esp32(Final version).zip
    └── note.txt
```

The file `docs/videos/video` is not referenced because it has no recognizable extension. It should be removed if it is a duplicate, or renamed with its correct extension.

The two dataset files appear to represent the same 42-row dataset. Since `train_model.py` currently reads `dataset_enfriamiento.csv`, that file should be treated as the active dataset. If both CSV files are identical, keeping only `dataset_enfriamiento.csv` will avoid confusion.

---

## 4. Hardware Platform

| Component | Function |
|---|---|
| ESP32-S3 N16R8 | Sensor acquisition, BLE communication, embedded processing, and future actuator control |
| Raspberry Pi Zero 2 W | BLE client, temporal capture, AI inference, and decision-making |
| MLX90614 | Infrared measurement of object and ambient temperature |
| Custom PCB | Physical integration of the ESP32-S3 and the supporting connections |
| USB Type-C adapter | Programming and power interface |

The MLX90614 provides:

```text
Tobj = object temperature
Tamb = ambient temperature
dT   = Tobj - Tamb
```

The custom PCB files are available in [hardware/esp32(Final version).zip](hardware/esp32(Final%20version).zip).

### PCB design

![PCB design](docs/images/PCB%20design.jpeg)

### Assembled prototype

![Assembled prototype](docs/images/Prototipo.jpeg)

---

## 5. Standalone ESP32 Prototype

The file `firmware/main.ino` contains the first complete implementation. It performs the sensor acquisition and thermal analysis directly on the ESP32-S3.

The firmware calculates the raw difference `dT = Tobj - Tamb` and applies an exponential filter controlled by `ALPHA_DT`. A measurement cycle starts only after `dT` remains above the start threshold for several consecutive samples. This reduces false starts caused by noise.

After the event begins, the firmware stores `Tobj_0`, `Tamb_0`, `dT_0`, `dT_5`, `dT_10`, `dT_20`, and `dT_30`. These values form the thermal input vector used by the AI model.

The same firmware implements a physical cooling estimate. A moving window of `dT` values is transformed with the natural logarithm. Linear regression estimates the slope of the logarithmic decay, and the thermal time constant is calculated from that slope. The time constant is then used to estimate the remaining time until `dT = 5 °C`.

The serial output is formatted as CSV-like data so that the run can be recorded and analyzed later.

---

## 6. AI Training

The training code is stored in `ai/train_model.py`. It loads `dataset_enfriamiento.csv`, separates the eight model inputs from the target output, normalizes the data, shuffles the samples, performs an 80/20 training and testing split, creates the neural network, trains it, compares predictions against real values, and converts the trained Keras model to TensorFlow Lite.

The network architecture is:

```text
Input(8)
   ↓
Dense(16, ReLU)
   ↓
Dense(8, ReLU)
   ↓
Dense(1, Linear)
```

The model uses Adam as optimizer, mean squared error as loss, and mean absolute error as an evaluation metric.

Input normalization is defined as:

```text
X_norm = (X - X_mean) / X_std
```

The normalized output is returned to seconds using:

```text
prediction_seconds = prediction_norm × y_std + y_mean
```

The current `dataset_enfriamiento.csv` contains 42 experiments, with 14 samples for morning, 14 for afternoon, and 14 for night.

Running the training script can generate `arduino_model.tflite`. That generated file is not currently committed to this repository. The final Raspberry Pi program cannot execute inference until a compatible `.tflite` file is generated and placed beside `raspberry_ai_client_final.py`, or until the model path in the Python code is changed.

---

## 7. Initial BLE Prototype

The prototype files are:

```text
ble/raspberry/mlx_ble.py
ble/esp32/ble_client.ino
```

In this version, the Raspberry Pi acts as a BLE server named `raspberry_kevin`. It reads the MLX90614 through I2C and publishes messages such as:

```text
OBJ:26.15,AMB:24.80
```

The ESP32 scans for the server, connects to the configured BLE service, subscribes to notifications, and prints the received message in the serial monitor.

This version validates BLE communication, but it is not the final architecture. Full details are available in [docs/ble-raspberry-esp32.md](docs/ble-raspberry-esp32.md).

---

## 8. Mini-Project 3

Mini-Project 3 focuses on the AI, IoT, firmware, and software components required to execute thermal prediction on the Raspberry Pi Zero 2 W.

The project presentation reported a previous dataset version with 22 samples. The current repository contains an expanded 42-sample dataset. The documentation distinguishes the historical presentation result from the current repository state.

The Raspberry Pi is expected to receive temperature measurements, capture the temporal variables, normalize the eight-value input vector, execute a previously trained TensorFlow Lite model, and convert the output back to seconds. The model is not retrained during normal operation.

Detailed information is available in [docs/miniproject-3.md](docs/miniproject-3.md).

---

## 9. Final Project Architecture

The final Arduino code should be stored as:

```text
ble/esp32/esp32_ble_server_final.ino
```

It reads the MLX90614 using SDA on GPIO 47 and SCL on GPIO 48. It creates the BLE service, publishes `Tobj,Tamb` once per second through a notify characteristic, and receives data through a write characteristic.

The final Raspberry Pi code should be stored as:

```text
ble/raspberry/raspberry_ai_client_final.py
```

It searches for `ESP32_SENSOR`, subscribes to the temperature characteristic, processes the received samples asynchronously, constructs the temporal input vector, executes TensorFlow Lite inference, and sends the remaining-time prediction back to the ESP32.

A critical integration detail remains: the Raspberry Pi sends a numerical string such as `481.68`, while the current ESP32 callback only turns the LED on or off when it receives `LED_ON` or `LED_OFF`. Therefore, BLE communication can succeed while the LED does nothing. The final control behavior must be defined consistently on both devices.

Full technical details are available in [docs/final-project.md](docs/final-project.md).

---

## 10. Dataset and Model Variables

| Variable | Description |
|---|---|
| `momento_dia` | 0 = morning, 1 = afternoon, 2 = night |
| `Tobj_0` | Initial object temperature |
| `Tamb_0` | Initial ambient temperature |
| `dT_0` | Initial thermal difference |
| `dT_5` | Thermal difference at 5 seconds |
| `dT_10` | Thermal difference at 10 seconds |
| `dT_20` | Thermal difference at 20 seconds |
| `dT_30` | Thermal difference at 30 seconds |
| `tiempo_hasta_dt5` | Total time required to reach `dT = 5 °C` |

The model learns the relationship:

```text
f(momento_dia, Tobj_0, Tamb_0, dT_0, dT_5, dT_10, dT_20, dT_30)
    = tiempo_hasta_dt5
```

The model file, the means, and the standard deviations must always come from the same training run. The normalization constants currently written in `raspberry_ai_client_final.py` do not match the statistics of the current 42-sample dataset. A new TensorFlow Lite model and updated constants must therefore be generated together before final validation.

---

## 11. Experimental Methodology

The thermal experiments were performed indoors. Data were collected during morning, afternoon, and night conditions. Ceramic surfaces were selected because their temperature changed slowly enough to capture the required 30-second input window and the later cooling time.

The MLX90614 was positioned above the object and aimed toward its center. During stove-based tests, the surrounding environment also heated up and the measured ambient temperature increased significantly. This reduced the quality of the temperature difference used by the model.

A lighter was later used for demonstrations because it provided lower and more controllable temperatures. The experiment still remains sensitive to sensor distance, surface emissivity, airflow, heat-source consistency, and ambient heating.

---

## 12. Experimental Evidence

The current dataset is available at [ai/dataset_enfriamiento.csv](ai/dataset_enfriamiento.csv). The serial log from a complete run is available at [docs/logs/test-run-01.txt](docs/logs/test-run-01.txt).

The short repository video is available at [docs/videos/Diseño final.mp4](docs/videos/Dise%C3%B1o%20final.mp4).

A longer external demonstration is available here:

[Watch the external demo video](https://drive.google.com/file/d/1Ku_SDlaufHtSQT0y7ulRh_jtI_WYhIBA/view?usp=sharing)

---

## 13. Current Status

| Component | Status | Result |
|---|---|---|
| MLX90614 acquisition | Completed | Object and ambient temperatures are read through I2C |
| Standalone ESP32 firmware | Completed | Filtering, physical estimation, temporal capture, and embedded prediction prototype |
| Custom PCB | Manufactured and tested | Functional with identified connection and layout issues |
| Dataset | Available | 42 cooling experiments in the current version |
| Keras training | Completed | Regression model architecture and training pipeline implemented |
| TensorFlow Lite conversion code | Available | The script can generate a `.tflite` file |
| TensorFlow Lite file in repository | Not included | Must be generated before running the final Raspberry client |
| Initial BLE prototype | Completed | Raspberry-server to ESP32-client notifications verified |
| Final ESP32 BLE server code | Available | Temperature notifications and control characteristic implemented |
| Final Raspberry AI client code | Available | Temporal capture and inference pipeline implemented |
| Final end-to-end inference | Pending repository validation | Requires the matching `.tflite` model and normalization values |
| LED/control policy | Not finalized | Raspberry sends a number, while ESP32 currently expects text commands |

---

## 14. Hardware Issues and Recommendations

The removable USB Type-C adapter was connected through a female header so that it could be replaced if the PCB failed. In practice, this removable connection became a source of unreliable operation. A future board should solder the adapter directly or use a more robust connector footprint.

The ESP32-S3 antenna area requires careful interpretation. The antenna region should be mechanically supported, but copper, ground planes, tracks, and components must not be placed inside the antenna keep-out area. Future revisions should follow the module manufacturer's antenna-layout recommendations.

Ground connections between layers were implemented using additional holes and male-to-male headers. A redesigned PCB should use plated through vias and ground stitching instead of external header connections.

---

## 15. Limitations

The dataset was collected under a limited set of indoor conditions. Ambient heating, sensor positioning, surface emissivity, and heat-source variation were not fully controlled. These effects reduce the physical consistency of the training data and can lead to incoherent predictions.

The repository does not currently include the TensorFlow Lite model required by the final Raspberry Pi program. The normalization constants in the final Python code also belong to a different training version than the current 42-sample CSV.

The first BLE prototype and the final BLE architecture use opposite server and client roles. Both are retained because they document the development process, but they must not be treated as the same implementation.

---

## 16. Reproduction Guide

### AI training

Install the Python packages manually:

```bash
pip install numpy pandas tensorflow matplotlib
```

Run the training script from the `ai` folder:

```bash
python train_model.py
```

The script expects `dataset_enfriamiento.csv`. After training, it generates the model and prints the normalization parameters. Copy the generated `arduino_model.tflite` beside `ble/raspberry/raspberry_ai_client_final.py`, then update the constants in that file using the values printed by the same training run.

### Standalone ESP32 firmware

Open `firmware/main.ino` in Arduino IDE, install the ESP32 board support, Adafruit MLX90614, and the required TensorFlow Lite Micro library, select the ESP32-S3 board and port, upload the sketch, and open the serial monitor at 115200 baud.

### Initial BLE prototype

On Raspberry Pi, install the required packages manually:

```bash
pip install bless adafruit-blinka adafruit-circuitpython-mlx90614
```

Run `ble/raspberry/mlx_ble.py`, upload `ble/esp32/ble_client.ino`, and open the ESP32 serial monitor.

### Final architecture

On Raspberry Pi, install:

```bash
pip install numpy tensorflow bleak
```

Upload `ble/esp32/esp32_ble_server_final.ino`. Place the compatible `arduino_model.tflite` beside `ble/raspberry/raspberry_ai_client_final.py`, update `MOMENTO_DIA` and the normalization constants, then execute:

```bash
python ble/raspberry/raspberry_ai_client_final.py
```

---

## 17. Future Work

The next step is to regenerate a TensorFlow Lite model from the current dataset and update the final Raspberry Pi normalization constants from the same training run. The complete BLE cycle should then be validated from temperature acquisition to prediction return.

The control protocol should be defined explicitly. The Raspberry Pi can either send a numeric remaining time and let the ESP32 apply a threshold, or send commands such as `LED_ON` and `LED_OFF`. Both programs must use the same protocol.

Future experiments should improve environmental control, repeatability, sensor positioning, and dataset size. The AI prediction should be compared quantitatively with the real time and with the physical `tau`-based estimate.

---

## 18. Conclusions

The project demonstrates a complete development path from custom hardware and thermal acquisition to neural-network training, BLE communication, and distributed AI inference.

The standalone ESP32 firmware validated local thermal processing. The initial BLE prototype validated communication between the Raspberry Pi and ESP32. The final code establishes the intended distributed architecture in which the ESP32-S3 handles sensing and BLE publication while the Raspberry Pi captures the temporal features and executes TensorFlow Lite inference.

The remaining work is primarily integration and validation: generating a model that matches the current dataset and normalization values, defining a consistent prediction/control message, and measuring the real prediction error under controlled conditions.

