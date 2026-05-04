# Embedded-AI-System-with-ESP32

Embedded AI system based on ESP32, including custom PCB design, firmware development, temperature sensing, and on-device inference with TensorFlow Lite.

## Overview

This project presents the design and implementation of an embedded intelligent system based on an ESP32 microcontroller. The system combines a custom PCB developed in KiCad, firmware written in C/C++, infrared temperature sensing with the MLX90614 sensor, and a TensorFlow Lite for Microcontrollers model running locally on the device. The main goal is to acquire thermal data, process it in real time, and estimate a target time-related thermal behavior directly on the ESP32 without relying on external computation.

## General Description

The project was developed as a complete embedded solution that integrates both hardware and software. On the hardware side, a custom PCB was designed for the ESP32-based system. On the software side, firmware was implemented to read temperature data from the MLX90614 sensor, process the thermal response of the observed object, estimate physical parameters from the measured data, and execute an AI model stored in `model.h` using TensorFlow Lite Micro. The system also prints structured serial output for testing and analysis.

## Main Features

- Custom PCB design for the ESP32-based embedded system
- Real-time object and ambient temperature acquisition using the MLX90614 sensor
- Thermal signal filtering and processing
- Physical-model-based estimation using a time constant (`tau`)
- AI-based prediction using TensorFlow Lite for Microcontrollers
- Serial output for monitoring, debugging, and CSV-style data logging
- Fully local execution on the ESP32 device without cloud processing

## Repository Structure

```text
.
├── README.md
├── firmware/
│   ├── main.ino
│   └── model.h
└── hardware/
    ├── note.txt
    └── esp32(Version final).zip
