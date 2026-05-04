#include <Wire.h>
#include <Adafruit_MLX90614.h>

#include <TensorFlowLite_ESP32.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model.h"

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// 0 = mañana, 1 = tarde, 2 = noche
const int MOMENTO_DIA = 0;

// Modelo físico
const int WINDOW = 20;
const float ALPHA_DT = 0.2;
const float ALPHA_TAU = 0.1;

const float START_DT = 3.0;
const float STOP_DT = 5.0;
const float LOW_DT_UNRELIABLE = 5.0;
const float LOCK_DT = 10.0;
const float EPSILON = 5.0;

float dT_buffer[WINDOW];
float ln_buffer[WINDOW];

int nSamples = 0;
int sampleNumber = 0;

float dT_filt = 0.0;
float tau_filt = 0.0;
float tau_final = 0.0;

bool started = false;
bool tau_locked = false;
int startConfirm = 0;

unsigned long startTime = 0;
float tiempo_seg = 0.0;

// ================= IA =================

float X_mean[8] = {
  1.0,
  75.12667,
  24.159998,
  51.755417,
  50.604168,
  48.944996,
  45.467915,
  42.388336
};

float X_std[8] = {
  0.8164966,
  11.085261,
  1.6978811,
  11.810352,
  11.608148,
  11.27286,
  10.421074,
  9.825294
};

float y_mean = 584.125;
float y_std = 140.1292;

// Datos que usará la IA
float Tobj_0 = 0;
float Tamb_0 = 0;
float dT_0 = 0;
float dT_5 = 0;
float dT_10 = 0;
float dT_20 = 0;
float dT_30 = 0;

bool capturo_0 = false;
bool capturo_5 = false;
bool capturo_10 = false;
bool capturo_20 = false;
bool capturo_30 = false;
bool ia_predicha = false;

// TensorFlow Lite
tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

const tflite::Model* tflModel = nullptr;
tflite::AllOpsResolver resolver;
tflite::MicroInterpreter* interpreter = nullptr;

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

constexpr int kTensorArenaSize = 20 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// ================= FUNCIONES IA =================

void setupIA() {
  tflModel = tflite::GetModel(model);

  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Error: version del modelo incompatible");
    while (1);
  }

  static tflite::MicroInterpreter static_interpreter(
    tflModel,
    resolver,
    tensor_arena,
    kTensorArenaSize,
    error_reporter
  );

  interpreter = &static_interpreter;

  TfLiteStatus allocate_status = interpreter->AllocateTensors();

  if (allocate_status != kTfLiteOk) {
    Serial.println("Error al reservar memoria para TensorFlow Lite");
    while (1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("IA lista");
}

float predecirIA() {
  float X[8] = {
    (float)MOMENTO_DIA,
    Tobj_0,
    Tamb_0,
    dT_0,
    dT_5,
    dT_10,
    dT_20,
    dT_30
  };

  for (int i = 0; i < 8; i++) {
    input->data.f[i] = (X[i] - X_mean[i]) / X_std[i];
  }

  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    Serial.println("Error ejecutando IA");
    return -1;
  }

  float pred_norm = output->data.f[0];
  float tiempo_predicho = pred_norm * y_std + y_mean;

  if (tiempo_predicho < 0) tiempo_predicho = 0;

  return tiempo_predicho;
}

// ================= FUNCIONES MODELO FISICO =================

void printCSVHeader() {
  Serial.println("momento_dia,muestra,tiempo_seg,Tobj,Tamb,dT,tau_inst,tau_uso,tiempo_restante,zona");
}

void resetRun() {
  nSamples = 0;
  sampleNumber = 0;
  dT_filt = 0.0;
  tau_filt = 0.0;
  tau_final = 0.0;
  started = false;
  tau_locked = false;
  startConfirm = 0;
  startTime = 0;
  tiempo_seg = 0.0;

  capturo_0 = false;
  capturo_5 = false;
  capturo_10 = false;
  capturo_20 = false;
  capturo_30 = false;
  ia_predicha = false;

  Tobj_0 = 0;
  Tamb_0 = 0;
  dT_0 = 0;
  dT_5 = 0;
  dT_10 = 0;
  dT_20 = 0;
  dT_30 = 0;

  Serial.println("FIN_CORRIDA");
  Serial.println();
  printCSVHeader();
}

void pushSample(float dT) {
  if (nSamples < WINDOW) {
    dT_buffer[nSamples] = dT;
    ln_buffer[nSamples] = log(dT);
    nSamples++;
  } else {
    for (int i = 0; i < WINDOW - 1; i++) {
      dT_buffer[i] = dT_buffer[i + 1];
      ln_buffer[i] = ln_buffer[i + 1];
    }

    dT_buffer[WINDOW - 1] = dT;
    ln_buffer[WINDOW - 1] = log(dT);
  }
}

bool computeTau(float &tau_inst) {
  if (nSamples < WINDOW) return false;

  float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

  for (int i = 0; i < WINDOW; i++) {
    float x = i;
    float y = ln_buffer[i];

    sumX += x;
    sumY += y;
    sumXY += x * y;
    sumX2 += x * x;
  }

  float denom = WINDOW * sumX2 - sumX * sumX;
  if (denom == 0) return false;

  float m = (WINDOW * sumXY - sumX * sumY) / denom;

  if (m >= -0.0005) return false;

  tau_inst = -1.0 / m;
  return true;
}

void printCSV(float Tobj, float Tamb, float dT, float tau_inst, float tau_uso, float tiempo_restante, String zona) {
  Serial.print(MOMENTO_DIA);
  Serial.print(",");
  Serial.print(sampleNumber);
  Serial.print(",");
  Serial.print(tiempo_seg, 2);
  Serial.print(",");
  Serial.print(Tobj, 2);
  Serial.print(",");
  Serial.print(Tamb, 2);
  Serial.print(",");
  Serial.print(dT, 2);
  Serial.print(",");

  if (tau_inst > 0) Serial.print(tau_inst, 2);
  else Serial.print("NA");

  Serial.print(",");

  if (tau_uso > 0) Serial.print(tau_uso, 2);
  else Serial.print("NA");

  Serial.print(",");

  if (tiempo_restante >= 0) Serial.print(tiempo_restante, 2);
  else Serial.print("NA");

  Serial.print(",");
  Serial.println(zona);
}

// ================= SETUP =================

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 7);

  if (!mlx.begin()) {
    Serial.println("Error MLX90614");
    while (1);
  }

  setupIA();

  printCSVHeader();
}

// ================= LOOP =================

void loop() {
  float Tobj = mlx.readObjectTempC();
  float Tamb = mlx.readAmbientTempC();
  float dT_raw = Tobj - Tamb;

  if (dT_filt == 0.0) {
    dT_filt = dT_raw;
  } else {
    dT_filt = ALPHA_DT * dT_raw + (1.0 - ALPHA_DT) * dT_filt;
  }

  if (!started) {
    if (dT_filt > START_DT) {
      startConfirm++;
    } else {
      startConfirm = 0;
    }

    if (startConfirm >= 3) {
      started = true;
      nSamples = 0;
      sampleNumber = 0;
      tau_filt = 0.0;
      tau_final = 0.0;
      tau_locked = false;
      startTime = millis();

      Serial.println("OBJETO DETECTADO - INICIANDO MEDICION");
    }

    delay(1000);
    return;
  }

  tiempo_seg = (millis() - startTime) / 1000.0;

  // Captura de variables para IA
  if (!capturo_0) {
    Tobj_0 = Tobj;
    Tamb_0 = Tamb;
    dT_0 = dT_filt;
    capturo_0 = true;

    Serial.println("IA: capturado punto 0 s");
  }

  if (!capturo_5 && tiempo_seg >= 5.0) {
    dT_5 = dT_filt;
    capturo_5 = true;

    Serial.println("IA: capturado dT_5");
  }

  if (!capturo_10 && tiempo_seg >= 10.0) {
    dT_10 = dT_filt;
    capturo_10 = true;

    Serial.println("IA: capturado dT_10");
  }

  if (!capturo_20 && tiempo_seg >= 20.0) {
    dT_20 = dT_filt;
    capturo_20 = true;

    Serial.println("IA: capturado dT_20");
  }

  if (!capturo_30 && tiempo_seg >= 30.0) {
    dT_30 = dT_filt;
    capturo_30 = true;

    Serial.println("IA: capturado dT_30");
  }

  if (capturo_30 && !ia_predicha) {
    float tiempoIA = predecirIA();
    float tiempoRestanteIA = tiempoIA - tiempo_seg;
    if (tiempoRestanteIA < 0) tiempoRestanteIA = 0;

    Serial.println();
    Serial.println("================================");
    Serial.println("PREDICCION IA");
    Serial.print("Tiempo total estimado hasta dT <= 5 C: ");
    Serial.print(tiempoIA, 2);
    Serial.println(" segundos");

    Serial.print("Tiempo restante estimado: ");
    Serial.print(tiempoRestanteIA, 2);
    Serial.println(" segundos");
    Serial.println("================================");
    Serial.println();

    ia_predicha = true;
  }


  if (dT_filt <= STOP_DT) {
    resetRun();
    delay(1000);
    return;
  }

  pushSample(dT_filt);
  sampleNumber++;

  if (nSamples < WINDOW) {
    printCSV(Tobj, Tamb, dT_filt, -1, -1, -1, "capturando");
    delay(1000);
    return;
  }

  float tau_inst = 0.0;
  bool ok = computeTau(tau_inst);

  if (dT_filt < LOW_DT_UNRELIABLE) {
    float tau_use = tau_locked ? tau_final : tau_filt;
    float tiempo_restante = -1;

    if (tau_use > 0) {
      tiempo_restante = tau_use * log(dT_filt / EPSILON);
      if (tiempo_restante < 0) tiempo_restante = 0;
    }

    printCSV(Tobj, Tamb, dT_filt, -1, tau_use, tiempo_restante, "dT_bajo");
    delay(1000);
    return;
  }

  if (ok) {
    if (tau_filt == 0.0) {
      tau_filt = tau_inst;
    } else {
      tau_filt = ALPHA_TAU * tau_inst + (1.0 - ALPHA_TAU) * tau_filt;
    }

    if (!tau_locked && dT_filt < LOCK_DT) {
      tau_final = tau_filt;
      tau_locked = true;
    }

    float tau_use = tau_locked ? tau_final : tau_filt;
    float tiempo_restante = tau_use * log(dT_filt / EPSILON);

    if (tiempo_restante < 0) tiempo_restante = 0;

    if (tau_locked) {
      printCSV(Tobj, Tamb, dT_filt, tau_inst, tau_use, tiempo_restante, "lock");
    } else {
      printCSV(Tobj, Tamb, dT_filt, tau_inst, tau_use, tiempo_restante, "normal");
    }

  } else {
    printCSV(Tobj, Tamb, dT_filt, -1, -1, -1, "regresion_no_valida");
  }

  delay(1000);
}
