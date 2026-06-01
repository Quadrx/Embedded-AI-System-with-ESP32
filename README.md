# Embedded-AI-System-with-ESP32

Embedded AI system based on ESP32-S3, including custom PCB design, firmware development, thermal sensing, AI model training, and on-device inference with TensorFlow Lite.

## Overview

This project presents the design and implementation of an embedded intelligent thermal monitoring system based on the ESP32-S3 platform. The system combines a custom PCB designed in KiCad, firmware developed in C/C++, an MLX90614 infrared temperature sensor, and a neural network trained in Python and deployed on embedded hardware.

The main objective of the project is to acquire thermal data in real time, process the temperature evolution of an object, and estimate the time required for the temperature difference to reach a defined threshold. The prediction is performed directly on the ESP32 using a TensorFlow Lite model.

## General Description

The project was developed as a complete hardware-software solution.

On the hardware side, a custom PCB was designed to integrate the ESP32-S3 module and the required connections for the thermal sensing system. The hardware design files are included as a compressed KiCad project.

On the software side, two main codebases were developed:

1. **Embedded firmware (`firmware/main.ino`)**  
   This code runs on the ESP32-S3. It reads temperature data from the MLX90614 sensor, processes the thermal response, estimates physical parameters such as the thermal time constant `tau`, and performs embedded inference using a TensorFlow Lite model.

2. **AI training script (`ai/train_model.py`)**  
   This Python script loads the thermal dataset, normalizes the data, trains a neural network using TensorFlow/Keras, evaluates the prediction performance, and exports the trained model so it can later be converted into a format suitable for deployment on the ESP32.

The repository also includes the dataset used for training and a serial log showing the behavior of the system during a real test run.

## Main Features

- Custom PCB design for the ESP32-S3-based system
- Real-time temperature acquisition using the MLX90614 sensor
- Thermal signal filtering and processing
- Physical-model-based estimation using the thermal time constant `tau`
- AI model training in Python using TensorFlow/Keras
- On-device inference on ESP32 using TensorFlow Lite
- Serial logging for validation and monitoring
- Experimental dataset included in the repository
- External demo video showing project operation

## Repository Structure

```text
.
├── README.md
├── ai/
│   ├── dataset_cooling.csv
│   └── train_model.py
├── docs/
│   └── logs/
│       └── test-run-01.txt
├── firmware/
│   └── main.ino
└── hardware/
    ├── esp32(Final version).zip
    └── note.txt
