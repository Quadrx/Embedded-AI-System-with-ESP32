import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow.keras import layers
import matplotlib.pyplot as plt

df = pd.read_csv("dataset_enfriamiento.csv")  #Lee el .csv creado
df.head()

# Aca se separa entradas y salidas, X son los datos que ve la IA y Y es la respuesta correcta que debe aprender (tiempo_hasta_dt5).
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

# Normalización para que la IA aprenda mejor, al dejar todo en escala parecida.
X_mean = X.mean(axis=0)
X_std = X.std(axis=0)
X_norm = (X - X_mean) / X_std

y_mean = y.mean()
y_std = y.std()
y_norm = (y - y_mean) / y_std

np.random.seed(42)
idx = np.random.permutation(len(X_norm))
X_norm = X_norm[idx]
y_norm = y_norm[idx]

train_size = int(0.8 * len(X_norm))

X_train = X_norm[:train_size]
y_train = y_norm[:train_size]

X_test = X_norm[train_size:]
y_test = y_norm[train_size:]


# Este bloque dice 8 entradas, capa de 16 neuronas, capa de 8 neuronas, 1 salida
# Las 8 entradas son las del .csv, la salida es: tiempo_hasta_dt5
model = tf.keras.Sequential([
    layers.Dense(16, activation="relu", input_shape=(8,)),
    layers.Dense(8, activation="relu"),
    layers.Dense(1)
])

model.compile(
    optimizer="adam",
    loss="mse",
    metrics=["mae"]
)

history = model.fit(
    X_train,
    y_train,
    epochs=500,      # Para que el codigo revise el .csv varias veces
    batch_size=4,
    validation_data=(X_test, y_test),
    verbose=1
)

# Aca el modelo compara (tiempo real y el de la IA), para saber que tan bien quedó.
pred_norm = model.predict(X_test)
pred = pred_norm.flatten() * y_std + y_mean
real = y_test * y_std + y_mean

print("Predicción vs Real")
for p, r in zip(pred, real):
    print(f"IA: {p:.1f} s | Real: {r:.1f} s | Error: {abs(p-r):.1f} s")

print("\nX_mean =", X_mean)
print("X_std =", X_std)
print("y_mean =", y_mean)
print("y_std =", y_std)




converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

open("arduino_model.tflite", "wb").write(tflite_model)

!echo "const unsigned char model[] = {" > model.h
!cat arduino_model.tflite | xxd -i >> model.h
!echo "};" >> model.h
!echo "const int model_len = sizeof(model);" >> model.h
