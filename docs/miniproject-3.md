# Mini-Project 3: AI, IoT, Firmware, and Software

## 1. Scope

Mini-Project 3 develops the artificial-intelligence and IoT foundation of the thermal prediction system. Its purpose is to transform experimental cooling measurements into a regression model and prepare that model for execution on embedded or edge-computing hardware.

The main software elements are:

```text
ai/dataset_enfriamiento.csv
ai/train_model.py
firmware/main.ino
```

The current repository also contains `ai/dataset_cooling.csv`. The available copies contain the same 42 experiments. Since the training script reads `dataset_enfriamiento.csv`, that file should be considered the active dataset and the duplicate can be removed or renamed as a legacy copy.

---

## 2. Thermal Prediction Problem

The MLX90614 provides the object temperature and the ambient temperature:

```text
Tobj = object temperature
Tamb = ambient temperature
```

The code calculates the thermal difference:

```text
dT = Tobj - Tamb
```

The prediction target is the total time required for that difference to reach 5 °C:

```text
tiempo_hasta_dt5
```

The model uses the early evolution of the cooling curve to estimate the later total cooling time.

---

## 3. Dataset

The current `dataset_enfriamiento.csv` contains 42 complete cooling experiments and 9 columns. The three time-of-day classes are balanced:

| Time category | Encoded value | Samples |
|---|---:|---:|
| Morning | 0 | 14 |
| Afternoon | 1 | 14 |
| Night | 2 | 14 |

Each row represents one complete run.

| Column | Meaning |
|---|---|
| `momento_dia` | Time category used to represent morning, afternoon, or night |
| `Tobj_0` | Object temperature at the beginning of the run |
| `Tamb_0` | Ambient temperature at the beginning of the run |
| `dT_0` | Initial filtered thermal difference |
| `dT_5` | Filtered thermal difference at 5 seconds |
| `dT_10` | Filtered thermal difference at 10 seconds |
| `dT_20` | Filtered thermal difference at 20 seconds |
| `dT_30` | Filtered thermal difference at 30 seconds |
| `tiempo_hasta_dt5` | Total time from the beginning until `dT` reaches 5 °C |

The project presentation described an earlier dataset version containing 22 samples. That number should be preserved as a historical result of that stage. The GitHub repository now contains the expanded 42-sample version.

---

## 4. Detailed Explanation of `train_model.py`

### 4.1 Imported libraries

```python
import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras import layers
import matplotlib.pyplot as plt
```

`NumPy` manages numerical arrays and normalization. `pandas` reads the CSV and extracts its columns. `TensorFlow` and `Keras` define and train the neural network. `matplotlib` is imported for possible plots, although the current script does not yet create a graph.

### 4.2 Dataset loading

```python
df = pd.read_csv("dataset_enfriamiento.csv")
df.head()
```

The CSV is loaded into a DataFrame. `df.head()` is useful in a notebook because it displays the first rows and verifies that the column names and values were read correctly.

When the code is executed as a normal `.py` file, `df.head()` does not automatically print anything. It can be replaced with:

```python
print(df.head())
```

if terminal output is desired.

### 4.3 Input and target separation

The code extracts eight input columns into `X` and the target column into `y`:

```python
X = df[[
    "momento_dia",
    "Tobj_0",
    "Tamb_0",
    "dT_0",
    "dT_5",
    "dT_10",
    "dT_20",
    "dT_30"
]].values.astype(np.float32)

y = df["tiempo_hasta_dt5"].values.astype(np.float32)
```

`X` has shape `[number_of_runs, 8]`. Each row is one experiment and each column is one input feature. `y` contains one cooling-time value for each row.

The conversion to `float32` is important because TensorFlow Lite commonly uses 32-bit floating-point tensors.

### 4.4 Normalization

The code calculates one mean and one standard deviation for every input feature:

```python
X_mean = X.mean(axis=0)
X_std = X.std(axis=0)
X_norm = (X - X_mean) / X_std
```

The target is normalized independently:

```python
y_mean = y.mean()
y_std = y.std()
y_norm = (y - y_mean) / y_std
```

This transformation places the variables on comparable scales. It is essential because `momento_dia` only ranges from 0 to 2, while temperatures and cooling times can be much larger.

The inverse transformation for the output is:

```text
prediction_seconds = prediction_norm × y_std + y_mean
```

The values of `X_mean`, `X_std`, `y_mean`, and `y_std` are part of the deployed model. They must match the exact dataset and model used during training.

For the current 42-sample dataset, the calculated values are approximately:

```text
X_mean = [
  1.000000,
  72.963806,
  25.007141,
  48.407380,
  47.586666,
  46.184760,
  43.152855,
  40.231426
]

X_std = [
  0.816497,
  12.135606,
  1.846039,
  12.731888,
  12.428539,
  12.025522,
  11.040464,
  10.340449
]

y_mean = 576.5952
y_std  = 141.5908
```

These numbers should only be used with a model trained from this exact dataset and preprocessing pipeline.

### 4.5 Reproducible shuffling

```python
np.random.seed(42)
idx = np.random.permutation(len(X_norm))
X_norm = X_norm[idx]
y_norm = y_norm[idx]
```

The rows are shuffled so that the split does not preserve an accidental ordering from the CSV. The fixed seed makes the permutation repeatable.

### 4.6 Training and testing split

```python
train_size = int(0.8 * len(X_norm))

X_train = X_norm[:train_size]
y_train = y_norm[:train_size]

X_test = X_norm[train_size:]
y_test = y_norm[train_size:]
```

The first 80% of the shuffled data is used for training and the remaining 20% is used for testing. With only 42 samples, the testing set is small. Therefore, individual results can change noticeably if the split changes.

### 4.7 Neural-network architecture

```python
model = tf.keras.Sequential([
    layers.Dense(16, activation="relu", input_shape=(8,)),
    layers.Dense(8, activation="relu"),
    layers.Dense(1)
])
```

The first hidden layer receives eight values and produces sixteen learned activations. The second hidden layer compresses them into eight activations. The final linear neuron outputs one continuous value.

The ReLU activation is:

```text
ReLU(z) = max(0, z)
```

It allows the model to represent nonlinear thermal relationships instead of behaving like a single linear equation.

### 4.8 Compilation

```python
model.compile(
    optimizer="adam",
    loss="mse",
    metrics=["mae"]
)
```

Adam updates the network weights. Mean squared error penalizes large prediction errors more strongly. Mean absolute error is displayed because it is easier to interpret as an average absolute difference in the normalized domain.

### 4.9 Training

```python
history = model.fit(
    X_train,
    y_train,
    epochs=500,
    batch_size=4,
    validation_data=(X_test, y_test),
    verbose=1
)
```

The model processes the training set 500 times. A batch size of four updates the weights after every four training examples. Validation data is evaluated after each epoch.

Because the dataset is small, 500 epochs can cause overfitting. The training and validation losses should be plotted before claiming that the final epoch is the best model.

### 4.10 Evaluation in real units

```python
pred_norm = model.predict(X_test)
pred = pred_norm.flatten() * y_std + y_mean
real = y_test * y_std + y_mean
```

The model output and test targets are converted back to seconds. The script then prints the prediction, real value, and absolute error for every test sample.

### 4.11 TensorFlow Lite conversion

```python
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

open("arduino_model.tflite", "wb").write(tflite_model)
```

This converts the Keras model to TensorFlow Lite and writes the binary file. The repository currently does not contain that generated file.

The original commands that use `!echo`, `cat`, and `xxd` are notebook or shell commands. They work in environments such as Google Colab, but they are not standard Python syntax in a normal script. They are only required when generating `model.h` for direct ESP32 deployment. The final Raspberry architecture uses the `.tflite` file directly and does not require `model.h`.

---

## 5. Detailed Explanation of `firmware/main.ino`

The standalone firmware combines the MLX90614 acquisition, AI input capture, a physical cooling model, and TensorFlow Lite Micro inference on the ESP32-S3.

### Sensor acquisition

The code initializes I2C and creates an `Adafruit_MLX90614` object. Each loop reads object and ambient temperature, then calculates the raw difference.

### Exponential filtering

The filtered value is calculated as:

```text
dT_filtered = ALPHA_DT × dT_raw + (1 - ALPHA_DT) × previous_filtered_value
```

A value of `ALPHA_DT = 0.2` gives more weight to previous samples than to the newest measurement, reducing noise at the cost of a slower response.

### Start detection

The run does not begin immediately after one high measurement. `dT` must remain above `START_DT` for three consecutive samples. This is a simple debounce mechanism.

### Temporal feature capture

The code records the initial values and the filtered differences after 5, 10, 20, and 30 seconds. Once the complete vector exists, the AI model can produce a total-time prediction.

### Physical-model estimation

The code stores a moving window of 20 `dT` values. If exponential cooling is approximated by:

```text
dT(t) = dT0 × exp(-t / tau)
```

then:

```text
ln(dT(t)) = ln(dT0) - t / tau
```

Linear regression on `ln(dT)` estimates the slope `m`, and:

```text
tau = -1 / m
```

The estimated remaining time to reach `EPSILON = 5 °C` is:

```text
time_remaining = tau × ln(dT / EPSILON)
```

### TensorFlow Lite Micro inference

The firmware loads the model from `model.h`, allocates a tensor arena, normalizes the eight values, invokes the interpreter, and denormalizes the output. This version is separate from the final Raspberry-based inference architecture.

---

## 6. Relationship with the Final Project

Mini-Project 3 establishes the dataset, training process, normalization logic, model architecture, and standalone embedded behavior. The Final Project redistributes these tasks: the ESP32-S3 sends temperature measurements, and the Raspberry Pi executes the inference.

The repository does not currently include a generated `.tflite` file. Therefore, the training script is present and documented, but the final Raspberry Pi inference cannot be reproduced until the model is generated and placed in the expected location.

---

## 7. Result and Limitations

The project proves that the cooling-time problem can be represented as an eight-input regression task. It also establishes a complete conversion path from CSV measurements to a TensorFlow Lite model.

The main limitation is the dataset size and experimental variability. Forty-two samples are sufficient for a proof of concept, but not enough to demonstrate broad generalization. More repeated experiments under controlled conditions are required.
