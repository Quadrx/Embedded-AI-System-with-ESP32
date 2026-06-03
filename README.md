# Embedded-AI-System-with-ESP32

Embedded AI system based on ESP32-S3, including custom PCB design, firmware development, thermal sensing, AI model training, and on-device inference with TensorFlow Lite.

---

## Table of Contents

- [1. Project Overview](#1-project-overview)
- [2. Project Objectives](#2-project-objectives)
- [3. Repository Structure](#3-repository-structure)
- [4. System Description](#4-system-description)
- [5. Hardware Description](#5-hardware-description)
- [6. Firmware Description](#6-firmware-description)
- [7. AI Training Pipeline](#7-ai-training-pipeline)
- [8. Model Inputs and Output](#8-model-inputs-and-output)
- [9. Experimental Methodology](#9-experimental-methodology)
- [10. Experimental Evidence](#10-experimental-evidence)
- [11. Additional Module: BLE Communication Between Raspberry Pi and ESP32](#11-additional-module-ble-communication-between-raspberry-pi-and-esp32)
- [12. Hardware Issues and Recommendations](#12-hardware-issues-and-recommendations)
- [13. Limitations](#13-limitations)
- [14. How to Reproduce the Project](#14-how-to-reproduce-the-project)
- [15. Future Work](#15-future-work)
- [16. Conclusions](#16-conclusions)

---

## 1. Project Overview

This project presents the design and implementation of an embedded thermal monitoring system based on the ESP32-S3. The system combines a custom PCB designed in KiCad, firmware written in C/C++, thermal sensing with the MLX90614 infrared sensor, and a neural network trained in Python and prepared for embedded deployment through TensorFlow Lite.

The main goal is to monitor the thermal behavior of an object, process the temperature evolution in real time, and estimate the time required for the thermal difference to reach a target threshold.

---

## 2. Project Objectives

The main objectives of the project are:

1. Design a custom PCB for an ESP32-S3-based system.
2. Acquire object and ambient temperature using the MLX90614 sensor.
3. Process thermal behavior in real time on the ESP32-S3.
4. Estimate the cooling process using both a physical approach and an AI model.
5. Train a neural network from experimental thermal data.
6. Document the complete workflow from hardware design to embedded validation.

---

## 3. Repository Structure

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
│   └── logs/
│       └── test-run-01.txt
├── firmware/
│   └── main.ino
└── hardware/
    ├── esp32(Final version).zip
    └── note.txt
```

The repository is organized into five main areas. The `firmware` folder contains the embedded code that runs on the ESP32-S3 for thermal sensing, signal processing, and prediction support. The `ai` folder contains the dataset and Python training script used to build the neural network model. The `hardware` folder stores the compressed KiCad project and related notes for the custom PCB.

The `ble` folder contains an additional communication module in which a Raspberry Pi acts as a BLE server and an ESP32 acts as a BLE client. Finally, the `docs` folder stores supporting technical documentation and serial logs collected during testing.

## 4. System Description

The system is divided into three main parts:

### Thermal Sensing
The MLX90614 sensor measures:

- object temperature
- ambient temperature

From these values, the system computes:

```text
dT = Tobj - Tamb
```

### Embedded Processing
The ESP32-S3 reads the sensor data, filters the thermal signal, detects the beginning of a thermal event, captures relevant thermal checkpoints, and estimates thermal behavior.

### AI Prediction
A neural network trained in Python uses thermal features from the cooling process to predict the time until the temperature difference reaches 5 °C.

---

## 5. Hardware Description

The hardware is based on an ESP32-S3 and a custom PCB designed in KiCad. The system includes the ESP32-S3 module, the MLX90614 sensor interface, power and programming connections, and grounding points.

The hardware design files are included in:

- `hardware/esp32(Final version).zip`

This archive contains the KiCad project used during the development of the PCB.

---

## 6. Firmware Description

The file `firmware/main.ino` contains the embedded firmware running on the ESP32-S3.

Its main tasks are:

- initialize serial and I2C communication
- initialize the MLX90614 sensor
- read object and ambient temperature
- calculate the thermal difference `dT`
- filter the thermal signal
- detect the beginning of a measurement cycle
- capture values at 0 s, 5 s, 10 s, 20 s, and 30 s
- estimate thermal behavior using a physical model
- support AI-based prediction
- print CSV-like serial output for analysis

### Firmware Operation Summary

1. The ESP32-S3 initializes the sensor and communication interfaces.
2. The system continuously reads `Tobj` and `Tamb`.
3. The thermal difference `dT` is computed and filtered.
4. A valid event is detected once the temperature difference exceeds a threshold for several consecutive readings.
5. Thermal checkpoints are stored at fixed times.
6. A physical model estimates the thermal time constant `tau`.
7. The collected thermal checkpoints are used for AI-based prediction.
8. The system prints structured serial logs for validation.

---

## 7. AI Training Pipeline

The file `ai/train_model.py` contains the Python code used to train the neural network.

The script performs the following steps:

1. Load the dataset from `dataset_cooling.csv`
2. Select the input features and output target
3. Normalize the data
4. Shuffle the dataset
5. Split the data into training and testing subsets
6. Build a neural network with TensorFlow/Keras
7. Train the model
8. Evaluate prediction quality
9. Print normalization parameters for embedded deployment
10. Convert the trained model to TensorFlow Lite format when needed

### Model Architecture

The neural network uses:

- 8 input features
- 16 neurons in the first hidden layer
- 8 neurons in the second hidden layer
- 1 output neuron

This architecture is used as a regression model for estimating cooling time.

---

## 8. Model Inputs and Output

The neural network uses the following 8 input variables:

1. `momento_dia`
2. `Tobj_0`
3. `Tamb_0`
4. `dT_0`
5. `dT_5`
6. `dT_10`
7. `dT_20`
8. `dT_30`

The target output is:

- `tiempo_hasta_dt5`

This means the model learns to estimate the time required for the temperature difference to reach 5 °C based on the early thermal evolution of the system.

The model can be interpreted as learning a function of the form:

```text
f(momento_dia, Tobj_0, Tamb_0, dT_0, dT_5, dT_10, dT_20, dT_30) = tiempo_hasta_dt5
```

---

## 9. Experimental Methodology

The experiments were carried out indoors, in the same location, during both daytime and nighttime conditions. Airflow was not considered because the environment was relatively closed.

Ceramic surfaces were selected because their temperature changes were slower, making them more suitable for observing thermal response over time.

The sensor was positioned above the object and pointed toward its center. However, the surrounding environment heated more than expected, causing ambient temperature to increase significantly and affecting the consistency of the measurements.

For this reason, a lighter became a more practical heat source than the stove for demonstration purposes, since it produced lower and more controllable temperatures.

---

## 10. Experimental Evidence

The project includes several files that help document both the embedded thermal prediction workflow and the BLE communication stage. The thermal dataset used for model training is available in [`ai/dataset_cooling.csv`](ai/dataset_cooling.csv), while a real serial output log from the ESP32 test run is stored in [`docs/logs/test-run-01.txt`](docs/logs/test-run-01.txt).

The hardware design files are included in the compressed KiCad archive [`hardware/esp32(Final version).zip`](hardware/esp32(Final%20version).zip). In addition, a demonstration video showing the system in operation is available at the following link:

[Watch the demo video](https://drive.google.com/file/d/1Ku_SDlaufHtSQT0y7ulRh_jtI_WYhIBA/view?usp=sharing)


## 11. Additional Module: BLE Communication Between Raspberry Pi and ESP32

This repository also includes an additional Bluetooth Low Energy communication module that complements the thermal monitoring work. In this stage, the Raspberry Pi is connected to the MLX90614 sensor and acts as a BLE server. It reads the ambient and object temperature through I2C, formats the information as text, and continuously transmits it through a BLE characteristic.

The ESP32 acts as a BLE client. It scans nearby BLE devices, searches for the Raspberry Pi server, connects to it, subscribes to BLE notifications, and prints the received temperature values in the serial monitor. This creates a simple real-time wireless link between sensing on the Raspberry Pi side and monitoring on the ESP32 side.

The transmitted data uses a compact format such as `OBJ:26.15,AMB:24.80`, where `OBJ` represents the object temperature and `AMB` represents the ambient temperature. This approach makes the communication easy to inspect, debug, and extend.

The code for this module is available in `ble/raspberry/mlx_ble.py` and `ble/esp32/ble_client.ino`. A more detailed technical explanation is included in [`docs/ble-raspberry-esp32.md`](docs/ble-raspberry-esp32.md).

## 12. Hardware Issues and Recommendations

During the hardware implementation, some practical issues were identified.

### USB Type-C Adapter
A female header was used so that the Type-C adapter could be removed if the PCB failed. However, this connection became part of the problem and negatively affected system operation.

**Recommendation:** solder the Type-C adapter directly to the PCB in future revisions.

### ESP32-S3 Antenna Area
The antenna area of the ESP32-S3 N16R8 module was left floating in the final design.

**Recommendation:** provide better mechanical support for the antenna region in future PCB versions.

### Grounding
Additional holes were created for ground connections, and male-to-male headers were used to connect the upper layer to ground.

---

## 13. Limitations

The project worked as a proof of concept, but several limitations were observed:

- ambient temperature increased during measurement
- the dataset was collected in a limited environment
- thermal disturbances affected the measurements
- AI predictions were not always fully consistent with the expected physical behavior

As a result, the system is functional, but the prediction quality depends strongly on the measurement conditions and on the quality of the collected dataset.

---

## 14. How to Reproduce the Project

### Hardware
1. Extract `hardware/esp32(Final version).zip`
2. Review the KiCad files
3. Manufacture and assemble the PCB

### Firmware
1. Open `firmware/main.ino` in Arduino IDE
2. Install the required ESP32 and MLX90614 libraries
3. Select the correct board and serial port
4. Upload the firmware to the ESP32-S3
5. Open the serial monitor

### AI
1. Open `ai/train_model.py`
2. Make sure `dataset_cooling.csv` is available
3. Install Python dependencies:
   - numpy
   - pandas
   - tensorflow
   - matplotlib
4. Run the script
5. Review the training results and exported model data

---

## 15. Future Work

Possible improvements for future versions of the project include:

- revising the PCB design
- improving the USB Type-C connection
- improving antenna support
- collecting cleaner thermal data
- expanding the dataset
- refining the AI model
- generating and including the final embedded deployment files

---

## 16. Conclusions

This project successfully integrated custom PCB design, embedded firmware, thermal sensing, AI model training, and experimental validation around an ESP32-S3 platform.

From the hardware perspective, the project provided useful lessons about connector integration, grounding, and PCB design details. From the software perspective, it demonstrated a complete workflow for reading thermal data, processing it in real time, and training an AI-based thermal prediction model.

Although the experimental setup introduced disturbances that affected prediction quality, the project remains a solid proof of concept and a practical guide for building a similar embedded AI system.
