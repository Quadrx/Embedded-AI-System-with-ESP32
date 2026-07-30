import asyncio
import time
from pathlib import Path

import numpy as np
import tensorflow as tf
from bleak import BleakClient, BleakScanner


# =========================================================
# CONFIGURACIÓN BLE
# =========================================================

DEVICE_NAME = "ESP32_SENSOR"

SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0"
TEMP_UUID = "12345678-1234-5678-1234-56789abcdef1"
CONTROL_UUID = "12345678-1234-5678-1234-56789abcdef2"


# =========================================================
# CONFIGURACIÓN DEL EXPERIMENTO
# =========================================================

# 0 = mañana
# 1 = tarde
# 2 = noche
MOMENTO_DIA = 2

# El experimento comienza cuando dT supera 3 °C
# durante tres muestras consecutivas.
START_DT = 3.0
START_CONFIRM_SAMPLES = 3

# Se vuelve a permitir una nueva corrida cuando dT baja a 2 °C.
STOP_DT = 2.0

# Filtro aplicado a dT, igual al usado en el código de adquisición.
ALPHA_DT = 0.2


# =========================================================
# NORMALIZACIÓN DEL MODELO
# =========================================================

# Orden de las entradas:
#
# 0 momento_dia
# 1 Tobj_0
# 2 Tamb_0
# 3 dT_0
# 4 dT_5
# 5 dT_10
# 6 dT_20
# 7 dT_30

X_MEAN = np.array(
    [
        0.90909094,
        74.34818,
        24.279999,
        50.8559,
        49.72455,
        48.09227,
        44.700912,
        41.625908,
    ],
    dtype=np.float32,
)

X_STD = np.array(
    [
        0.7925271,
        11.081412,
        1.716548,
        11.802157,
        11.600937,
        11.266078,
        10.445874,
        9.787631,
    ],
    dtype=np.float32,
)

Y_MEAN = np.float32(560.11725)
Y_STD = np.float32(122.143166)


# =========================================================
# CLASE PARA CAPTURAR LAS MUESTRAS
# =========================================================

class CapturaEnfriamiento:

    def __init__(self):
        self.dT_filtrado = None
        self.reiniciar_corrida()

    def reiniciar_corrida(self):
        self.estado = "esperando"
        self.confirmaciones = 0
        self.tiempo_inicio = None

        self.Tobj_0 = None
        self.Tamb_0 = None
        self.dT_0 = None
        self.dT_5 = None
        self.dT_10 = None
        self.dT_20 = None
        self.dT_30 = None

    def actualizar_filtro(self, dT_raw):
        if self.dT_filtrado is None:
            self.dT_filtrado = dT_raw
        else:
            self.dT_filtrado = (
                ALPHA_DT * dT_raw
                + (1.0 - ALPHA_DT) * self.dT_filtrado
            )

        return self.dT_filtrado

    def procesar(self, Tobj, Tamb):
        ahora = time.monotonic()

        dT_raw = Tobj - Tamb
        dT = self.actualizar_filtro(dT_raw)

        print(
            f"Tobj={Tobj:.2f} °C | "
            f"Tamb={Tamb:.2f} °C | "
            f"dT={dT:.2f} °C | "
            f"estado={self.estado}"
        )

        # -------------------------------------------------
        # ESPERANDO EL INICIO DE UNA CORRIDA
        # -------------------------------------------------

        if self.estado == "esperando":

            if dT > START_DT:
                self.confirmaciones += 1
            else:
                self.confirmaciones = 0

            if self.confirmaciones >= START_CONFIRM_SAMPLES:
                self.estado = "capturando"
                self.tiempo_inicio = ahora

                self.Tobj_0 = Tobj
                self.Tamb_0 = Tamb
                self.dT_0 = dT

                print()
                print("========================================")
                print("INICIO DE CORRIDA")
                print(f"Tobj_0 = {self.Tobj_0:.2f}")
                print(f"Tamb_0 = {self.Tamb_0:.2f}")
                print(f"dT_0   = {self.dT_0:.2f}")
                print("========================================")
                print()

            return None

        tiempo_transcurrido = ahora - self.tiempo_inicio

        # -------------------------------------------------
        # ESPERAR UNA NUEVA CORRIDA DESPUÉS DE TERMINAR
        # -------------------------------------------------

        if self.estado == "predicho":

            if dT <= STOP_DT:
                print()
                print("dT llegó a la zona de finalización.")
                print("Sistema preparado para una nueva corrida.")
                print()

                self.reiniciar_corrida()

            return None

        # -------------------------------------------------
        # CAPTURA DE dT EN 5, 10, 20 Y 30 SEGUNDOS
        # -------------------------------------------------

        if self.dT_5 is None and tiempo_transcurrido >= 5.0:
            self.dT_5 = dT
            print(f"Muestra dT_5 capturada: {self.dT_5:.2f}")

        if self.dT_10 is None and tiempo_transcurrido >= 10.0:
            self.dT_10 = dT
            print(f"Muestra dT_10 capturada: {self.dT_10:.2f}")

        if self.dT_20 is None and tiempo_transcurrido >= 20.0:
            self.dT_20 = dT
            print(f"Muestra dT_20 capturada: {self.dT_20:.2f}")

        if self.dT_30 is None and tiempo_transcurrido >= 30.0:
            self.dT_30 = dT
            print(f"Muestra dT_30 capturada: {self.dT_30:.2f}")

        muestras_completas = all(
            valor is not None
            for valor in [
                self.Tobj_0,
                self.Tamb_0,
                self.dT_0,
                self.dT_5,
                self.dT_10,
                self.dT_20,
                self.dT_30,
            ]
        )

        if muestras_completas:
            self.estado = "predicho"

            entrada = np.array(
                [
                    MOMENTO_DIA,
                    self.Tobj_0,
                    self.Tamb_0,
                    self.dT_0,
                    self.dT_5,
                    self.dT_10,
                    self.dT_20,
                    self.dT_30,
                ],
                dtype=np.float32,
            )

            return entrada, tiempo_transcurrido

        return None


# =========================================================
# CARGAR MODELO TENSORFLOW LITE
# =========================================================

def cargar_modelo():
    carpeta_programa = Path(__file__).resolve().parent
    ruta_modelo = carpeta_programa / "arduino_model.tflite"

    if not ruta_modelo.exists():
        raise FileNotFoundError(
            f"No se encontró el modelo en: {ruta_modelo}"
        )

    print("Cargando modelo:", ruta_modelo)

    interpreter = tf.lite.Interpreter(
        model_path=str(ruta_modelo)
    )

    interpreter.allocate_tensors()

    entrada = interpreter.get_input_details()
    salida = interpreter.get_output_details()

    print("Modelo cargado correctamente")
    print("Forma de entrada:", entrada[0]["shape"])
    print("Forma de salida:", salida[0]["shape"])

    return interpreter, entrada, salida


# =========================================================
# EJECUTAR LA IA
# =========================================================

def ejecutar_modelo(
    interpreter,
    input_details,
    output_details,
    datos,
):
    datos_normalizados = (datos - X_MEAN) / X_STD

    entrada_modelo = np.expand_dims(
        datos_normalizados,
        axis=0,
    ).astype(np.float32)

    interpreter.set_tensor(
        input_details[0]["index"],
        entrada_modelo,
    )

    interpreter.invoke()

    salida_normalizada = interpreter.get_tensor(
        output_details[0]["index"]
    )

    prediccion_normalizada = float(
        salida_normalizada[0][0]
    )

    tiempo_total_predicho = (
        prediccion_normalizada * float(Y_STD)
        + float(Y_MEAN)
    )

    return tiempo_total_predicho


# =========================================================
# BUSCAR ESP32
# =========================================================

async def buscar_esp32():
    while True:
        print("Buscando ESP32_SENSOR...")

        dispositivos = await BleakScanner.discover(
            timeout=5.0
        )

        for dispositivo in dispositivos:
            nombre = dispositivo.name or ""

            print(
                f"Encontrado: {nombre} "
                f"[{dispositivo.address}]"
            )

            if nombre == DEVICE_NAME:
                print("ESP32_SENSOR encontrado")
                return dispositivo

        print("ESP32 no encontrado. Buscando nuevamente...")
        await asyncio.sleep(2)


# =========================================================
# PROCESAR NOTIFICACIONES
# =========================================================

async def procesar_notificaciones(
    cola,
    client,
    captura,
    interpreter,
    input_details,
    output_details,
):
    while True:
        datos_ble = await cola.get()

        try:
            texto = datos_ble.decode("utf-8").strip()

            partes = texto.split(",")

            if len(partes) != 2:
                print("Formato BLE incorrecto:", texto)
                continue

            Tobj = float(partes[0])
            Tamb = float(partes[1])

            resultado = captura.procesar(Tobj, Tamb)

            if resultado is None:
                continue

            vector_entrada, tiempo_transcurrido = resultado

            print()
            print("========================================")
            print("ENTRADA DE LA IA")
            print("========================================")
            print(f"momento_dia = {vector_entrada[0]:.0f}")
            print(f"Tobj_0      = {vector_entrada[1]:.2f}")
            print(f"Tamb_0      = {vector_entrada[2]:.2f}")
            print(f"dT_0        = {vector_entrada[3]:.2f}")
            print(f"dT_5        = {vector_entrada[4]:.2f}")
            print(f"dT_10       = {vector_entrada[5]:.2f}")
            print(f"dT_20       = {vector_entrada[6]:.2f}")
            print(f"dT_30       = {vector_entrada[7]:.2f}")

            tiempo_total = ejecutar_modelo(
                interpreter,
                input_details,
                output_details,
                vector_entrada,
            )

            # El modelo predice el tiempo total desde el comienzo
            # hasta llegar a dT = 5.
            #
            # Como la predicción se realiza después de aproximadamente
            # 30 segundos, se calcula también el tiempo restante.

            tiempo_restante = max(
                tiempo_total - tiempo_transcurrido,
                0.0,
            )

            print()
            print("========================================")
            print("RESULTADO DE LA IA")
            print("========================================")
            print(
                f"Tiempo total predicho: "
                f"{tiempo_total:.2f} segundos"
            )
            print(
                f"Tiempo transcurrido: "
                f"{tiempo_transcurrido:.2f} segundos"
            )
            print(
                f"Tiempo restante: "
                f"{tiempo_restante:.2f} segundos"
            )
            print("========================================")
            print()

            comando = f"{tiempo_restante:.2f}".encode(
                "utf-8"
            )

            await client.write_gatt_char(
                CONTROL_UUID,
                comando,
                response=True,
            )

            print(
                "Predicción enviada al ESP32:",
                comando.decode(),
            )

        except ValueError as error:
            print("Error convirtiendo temperatura:", error)

        except UnicodeDecodeError as error:
            print("Error decodificando BLE:", error)

        except Exception as error:
            print("Error procesando notificación:", error)


# =========================================================
# CONEXIÓN BLE
# =========================================================

async def conectar(
    dispositivo,
    interpreter,
    input_details,
    output_details,
):
    cola = asyncio.Queue()
    captura = CapturaEnfriamiento()
    loop = asyncio.get_running_loop()

    def notification_handler(sender, data):
        datos_copiados = bytes(data)

        loop.call_soon_threadsafe(
            cola.put_nowait,
            datos_copiados,
        )

    print("Conectando al ESP32...")

    async with BleakClient(dispositivo) as client:
        print("Conectado:", client.is_connected)

        await client.start_notify(
            TEMP_UUID,
            notification_handler,
        )

        print("Suscrito a las temperaturas")
        print("Esperando datos...")
        print()

        tarea_procesamiento = asyncio.create_task(
            procesar_notificaciones(
                cola,
                client,
                captura,
                interpreter,
                input_details,
                output_details,
            )
        )

        try:
            while client.is_connected:
                if tarea_procesamiento.done():
                    tarea_procesamiento.result()

                await asyncio.sleep(1)

        finally:
            tarea_procesamiento.cancel()

            try:
                await tarea_procesamiento
            except asyncio.CancelledError:
                pass


# =========================================================
# PROGRAMA PRINCIPAL
# =========================================================

async def main():
    interpreter, input_details, output_details = (
        cargar_modelo()
    )

    while True:
        try:
            dispositivo = await buscar_esp32()

            await conectar(
                dispositivo,
                interpreter,
                input_details,
                output_details,
            )

            print("Conexión perdida. Reconectando...")

        except KeyboardInterrupt:
            raise

        except Exception as error:
            print("Error de conexión:", error)
            print("Intentando nuevamente en 3 segundos...")
            await asyncio.sleep(3)


if __name__ == "__main__":
    try:
        asyncio.run(main())

    except KeyboardInterrupt:
        print()
        print("Programa detenido por el usuario.")
