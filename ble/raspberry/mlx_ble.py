kevin@raspberrypi:~ $ cat mlx_ble.py 
import asyncio
import board
import busio
import adafruit_mlx90614

from bless import (
    BlessServer,
    GATTCharacteristicProperties,
    GATTAttributePermissions,
)

SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0"
CHAR_UUID    = "12345678-1234-5678-1234-56789abcdef1"

i2c = busio.I2C(board.SCL, board.SDA)
mlx = adafruit_mlx90614.MLX90614(i2c)

async def main():

    server = BlessServer(name="raspberry_kevin")

    await server.add_new_service(SERVICE_UUID)

    await server.add_new_characteristic(
        SERVICE_UUID,
        CHAR_UUID,
        properties=(
            GATTCharacteristicProperties.read
            | GATTCharacteristicProperties.notify
        ),
        value=bytearray(b"Inicio"),
        permissions=GATTAttributePermissions.readable,
    )

    await server.start()

    print("=================================")
    print("BLE iniciado")
    print("Nombre: raspberry_kevin")
    print("=================================")

    while True:

        amb = mlx.ambient_temperature
        obj = mlx.object_temperature

        dato = f"OBJ:{obj:.2f},AMB:{amb:.2f}"

        print(dato)

        try:
            char = server.get_characteristic(CHAR_UUID)

            if char is not None:
                char.value = bytearray(dato.encode())

                server.update_value(
                    SERVICE_UUID,
                    CHAR_UUID
                )

        except Exception as e:
            print("Error BLE:", e)

        await asyncio.sleep(1)

if __name__ == "__main__":
    asyncio.run(main())
