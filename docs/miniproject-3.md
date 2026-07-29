# Mini-Project 3: AI, IoT, Firmware, and Software on Raspberry Pi Zero 2 W

## Overview

Mini-Project 3 focuses on the implementation of artificial intelligence, IoT communication, firmware, and software using a Raspberry Pi Zero 2 W as the main processing platform.

The purpose of this stage is to prepare and verify the software components required for an intelligent thermal prediction system. The system observes the cooling behavior of an object and estimates the time required for the difference between the object temperature and the ambient temperature to reach 5 °C.

The artificial intelligence model is trained previously using TensorFlow/Keras. The Raspberry Pi does not train the model during normal system operation. Instead, it loads the converted TensorFlow Lite model and performs inference using the weights learned during training.

---

## Project Objective

The objective of Mini-Project 3 is to develop a processing pipeline capable of receiving thermal measurements, constructing the required input variables, executing a neural-network model, and producing a cooling-time prediction.

The software pipeline follows this general sequence:

```text
Temperature measurements
        ↓
Input feature extraction
        ↓
Normalization
        ↓
TensorFlow Lite inference
        ↓
Output denormalization
        ↓
Estimated time until dT = 5 °C
```

The Raspberry Pi Zero 2 W is intended to perform data reception, preprocessing, inference, and prediction transmission.

---

## Main Technologies

| Technology | Function |
|---|---|
| Raspberry Pi Zero 2 W | Data processing and AI inference |
| Python | Training and inference software |
| TensorFlow/Keras | Neural-network training |
| TensorFlow Lite | Execution of the trained model |
| Bluetooth Low Energy | Wireless communication |
| ESP32-S3 | Thermal data acquisition and communication |
| MLX90614 | Object and ambient temperature measurement |

---

## Thermal Prediction Problem

The MLX90614 provides two temperature measurements:

```text
Tobj = object temperature
Tamb = ambient temperature
```

The temperature difference is calculated as:

```text
dT = Tobj - Tamb
```

The model must estimate how long the object will take to reach:

```text
dT = 5 °C
```

The predicted variable is called:

```text
tiempo_hasta_dt5
```

---

## Dataset

The model was trained using data collected from cooling experiments. Each dataset row represents one complete cooling run and contains the initial thermal conditions, the evolution of the temperature difference, and the final cooling time.

According to the project presentation, the dataset version used during that stage contained 22 samples. The dataset currently stored in the repository should be considered the authoritative version if it has been updated since the presentation.

The dataset is available at:

[Cooling Dataset](../ai/dataset_cooling.csv)

---

## Model Variables

The neural network uses eight input variables and produces one numerical output.

| Variable | Description |
|---|---|
| `momento_dia` | Time-of-day category: 0 = morning, 1 = afternoon, 2 = night |
| `Tobj_0` | Object temperature at the beginning of the cooling sequence |
| `Tamb_0` | Ambient temperature at the beginning of the cooling sequence |
| `dT_0` | Initial temperature difference |
| `dT_5` | Temperature difference after 5 seconds |
| `dT_10` | Temperature difference after 10 seconds |
| `dT_20` | Temperature difference after 20 seconds |
| `dT_30` | Temperature difference after 30 seconds |
| `tiempo_hasta_dt5` | Time required to reach `dT = 5 °C` |

The learned relationship can be represented as:

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

---

## Neural-Network Architecture

The model is a fully connected feed-forward neural network.

```text
Input: 8 variables
        ↓
Dense layer: 16 neurons, ReLU
        ↓
Dense layer: 8 neurons, ReLU
        ↓
Output layer: 1 neuron, linear activation
```

The training configuration is:

| Parameter | Configuration |
|---|---|
| Model type | Feed-forward neural network |
| First hidden layer | 16 neurons with ReLU |
| Second hidden layer | 8 neurons with ReLU |
| Output layer | 1 linear neuron |
| Optimizer | Adam |
| Loss function | Mean Squared Error |
| Evaluation metric | Mean Absolute Error |

The training code is available at:

[AI Training Script](../ai/train_model.py)

---

## Data Normalization

Normalization is required because the input variables use different numerical scales.

Each input variable is normalized using:

```text
X_norm = (X - X_mean) / X_std
```

The target output is normalized using its own mean and standard deviation.

After inference, the normalized model output is converted back to seconds using:

```text
prediction_seconds = prediction_norm × y_std + y_mean
```

The normalization parameters used during inference must be identical to the parameters calculated during training. Any difference between the training and deployment parameters can produce incorrect predictions.

---

## Training Process

The training script performs the following operations:

1. Loads the cooling dataset.
2. Separates the eight input variables from the target output.
3. Calculates the mean and standard deviation of the data.
4. Normalizes the inputs and output.
5. Randomly shuffles the samples.
6. Divides the dataset into training and testing subsets.
7. Creates the neural network.
8. Trains the model using TensorFlow/Keras.
9. Compares predicted values against real cooling times.
10. Prints the normalization parameters.
11. Converts the trained model to TensorFlow Lite.

The model is trained before deployment. Training is not performed on the Raspberry Pi during normal operation.

---

## TensorFlow Lite Conversion

After training, the Keras model is converted to TensorFlow Lite and saved as:

```text
arduino_model.tflite
```

TensorFlow Lite produces a smaller and more portable model that can be executed on embedded or resource-constrained platforms.

The conversion process separates two stages:

```text
Training stage:
Dataset → TensorFlow/Keras → trained model

Deployment stage:
trained model → TensorFlow Lite → inference
```

The Raspberry Pi only executes the deployment stage.

---

## Raspberry Pi Inference Process

The expected inference flow on the Raspberry Pi Zero 2 W is:

1. Receive object and ambient temperatures through BLE.
2. Detect the beginning of a new cooling sequence.
3. Store `Tobj_0`, `Tamb_0`, and `dT_0`.
4. Capture `dT_5`, `dT_10`, `dT_20`, and `dT_30`.
5. Add the time-of-day category.
6. Construct the eight-value input vector.
7. Normalize each input using the training parameters.
8. Execute the TensorFlow Lite interpreter.
9. Read the normalized output.
10. Convert the output back to seconds.
11. Send the prediction to the ESP32 through BLE.

The TensorFlow Lite test performed on the Raspberry Pi confirmed that the model could be loaded correctly.

The interpreter recognized:

```text
Input tensor shape:  [1, 8]
Output tensor shape: [1, 1]
```

This confirms that the model expects one sample containing eight variables and produces one numerical prediction.

---

## IoT Communication Role

Bluetooth Low Energy is used to connect the Raspberry Pi and the ESP32.

In the target architecture, the Raspberry Pi acts as the BLE client. It connects to the ESP32-S3, subscribes to temperature notifications, receives approximately one temperature pair per second, and writes the final prediction back to the ESP32.

The Raspberry Pi is therefore responsible for both AI processing and high-level communication logic.

---

## Mini-Project Result

Mini-Project 3 demonstrated that:

- the thermal prediction problem could be represented using eight input variables;
- the neural network could be trained using TensorFlow/Keras;
- the trained model could be converted to TensorFlow Lite;
- the Raspberry Pi Zero 2 W could load the TFLite model;
- the interpreter correctly identified the model input and output dimensions;
- BLE could be used as the communication layer between the Raspberry Pi and the ESP32.

The remaining task is to combine real-time BLE acquisition, temporal feature capture, TensorFlow Lite inference, and return transmission in one continuous application.

---

## Conclusion

Mini-Project 3 established the software and AI foundation of the thermal prediction system.

The Raspberry Pi Zero 2 W was selected as the processing unit because it can receive measurements, preprocess data, execute TensorFlow Lite inference, and transmit the resulting prediction without retraining the model.

This stage confirms that the AI component is technically compatible with the distributed final architecture and prepares the project for complete ESP32–Raspberry Pi integration.
