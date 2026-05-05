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

## Code Operation

The firmware operates as a complete thermal monitoring and prediction pipeline running on the ESP32-S3. First, the system initializes serial communication, I2C communication, the MLX90614 infrared temperature sensor, and the TensorFlow Lite Micro interpreter. After initialization, the device continuously reads both object temperature and ambient temperature from the sensor.

From these measurements, the code computes the thermal difference:

`dT = Tobj - Tamb`

This thermal difference is filtered to reduce noise and improve stability. The system then waits until the filtered temperature difference exceeds a predefined threshold for several consecutive samples. This condition is used to detect the presence of the target object and to start a new measurement cycle.

Once the cycle starts, the code stores thermal values at specific time instants, including the initial conditions and the filtered thermal difference after 5, 10, 20, and 30 seconds. These values are used as input features for the embedded AI model. After enough data has been collected, the TensorFlow Lite model is executed and predicts the estimated total time associated with the observed thermal behavior.

At the same time, the code also applies a physical approach based on thermal decay. A moving window of samples is stored, transformed using the natural logarithm, and processed through linear regression to estimate the thermal time constant `tau`. This value is then used to estimate the remaining time until the thermal difference reaches a target threshold.

Finally, the firmware prints structured serial output in CSV-like format, including temperatures, elapsed time, thermal difference, estimated `tau`, remaining time, and the current operating state. This allows the system behavior to be monitored, validated, and analyzed experimentally.

## Experimental Notes and Hardware Recommendations

During hardware implementation, some practical issues were identified. One of the main problems was related to the USB Type-C adapter used to connect the ESP32 board to the computer. During assembly, a female header was used so that the adapter could be removed in case the PCB design did not work properly. However, this removable connection became part of the problem and negatively affected the system operation. Based on this experience, it is recommended to solder the Type-C adapter directly to the PCB instead of using detachable headers.

Another relevant issue in the final PCB design was related to the ESP32-S3 N16R8 module antenna area. In the implemented version, the antenna region was left floating. For future PCB manufacturing, it is recommended that the board material properly supports and covers the antenna area according to good mechanical design practice.

Since no major difficulties were found with the PCB layers, additional holes were created for ground connections. In order to connect the upper layer to ground, male-to-male header pins were soldered as a practical grounding solution.

To validate the code and the application itself, temperature data were collected during both daytime and nighttime conditions. Measurements were taken in the same indoor location, so external factors such as airflow were not considered because the environment was relatively closed and stable. Ceramic surfaces were selected for testing because their temperature changes occurred over a relatively longer period of time, making them more suitable for observing the system response.

## Measurement Conditions and Observations

An important aspect of the experimental setup is that the sensor measured the object from above while aiming at its center. However, the surrounding environment heated up more than expected, which caused the ambient temperature to rise to approximately 55 °C. This also heated nearby stove components and altered the measurement conditions.

Because of this effect, using the stove as the main heat source introduced disturbances in the thermal data. To better demonstrate the code operation, it became more practical to use a lighter as the heat source, since it produced lower and more controllable temperatures. Even so, these conditions affected the quality and consistency of the collected dataset.

## Limitations

The experimental conditions introduced limitations in the collected data. Since the ambient temperature increased significantly during measurement, the separation between object temperature and environmental influence was reduced. As a result, the thermal behavior used for training and validation was not always fully representative of an ideal controlled process.

For this reason, the AI model produced predictions that were not always fully coherent with the expected physical behavior. In other words, the embedded model was functional, but the quality of the results was strongly influenced by the experimental setup and by the thermal disturbances present during data acquisition.

## Conclusions

This project successfully integrated custom PCB design, embedded firmware development, infrared thermal sensing, and on-device AI inference using an ESP32-S3 platform. The implementation demonstrated that it is possible to run a complete thermal monitoring and prediction process locally on embedded hardware.

From the hardware perspective, the development process highlighted practical design lessons, especially regarding USB Type-C integration, grounding strategies, and PCB considerations around the ESP32-S3 antenna area. These observations are valuable for improving future versions of the board.

From the software perspective, the code achieved its main objective: reading temperature data, processing the thermal response, estimating physical parameters, and executing an AI model directly on the microcontroller. The system therefore provides a functional proof of concept for embedded intelligent thermal analysis.

However, the experiments also showed that the quality of the final predictions depends heavily on the data acquisition conditions. Uncontrolled environmental heating affected the measurements and reduced the coherence of the AI output. Therefore, future work should focus on improving the experimental setup, collecting cleaner data, and refining the training process so that the embedded model produces more reliable and physically consistent predictions.


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
