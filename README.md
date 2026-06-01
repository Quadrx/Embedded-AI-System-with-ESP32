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
- [11. Hardware Issues and Recommendations](#11-hardware-issues-and-recommendations)
- [12. Limitations](#12-limitations)
- [13. How to Reproduce the Project](#13-how-to-reproduce-the-project)
- [14. Future Work](#14-future-work)
- [15. Conclusions](#15-conclusions)

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
6. Document the complete workflow from hardware to embedded validation.

---

## 3. Repository Structure

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
