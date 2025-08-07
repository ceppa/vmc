# VMC MQTT Controller

This project is a C application designed to interface with a ventilation control system (VMC) via a UART serial connection, collect environmental sensor data, and communicate with an MQTT broker for home automation integration.

## Features

- Reads sensor values (temperature, airflow, pressure, etc.) from a VMC device using serial communication.
- Publishes sensor data to MQTT topics for external monitoring or control.
- Subscribes to a topic to receive speed-setting commands for the VMC system.
- Automatically retries connection to MQTT broker and resubscribes if the connection is lost.
- Supports basic signal handling for graceful shutdown.
- Minimal latency between read cycles and command transmissions.

## Technologies

- **Language**: C
- **Libraries**:
  - [Paho MQTT C Async](https://www.eclipse.org/paho/index.php?page=clients/c/index.php) for MQTT communication
  - `DEV_Config.h` for hardware-specific UART and GPIO setup
- **Hardware**: Intended for systems with UART capabilities (e.g., Raspberry Pi with SC0 serial interface)


## Configuration

The client uses a configuration file `mqtt.cfg` located in the same directory as the executable. The file format is:

```

username={USERNAME}
password={PASSWORD}

````

- On the first run, if `mqtt.cfg` is missing, it will be created with default placeholders:
  - username=`{USERNAME}`
  - password=`{PASSWORD}`
- Edit `mqtt.cfg` and replace these placeholders with your actual MQTT broker credentials before running the client again.

## Build Instructions

You'll need:
- GCC or compatible C compiler
- The Paho MQTT C library (Async version)
- `DEV_Config.h` and corresponding implementation for your device

```bash
gcc -o vmc_mqtt vmc_mqtt.c -lpaho-mqtt3as
````

Or, if you have other dependencies:

```bash
gcc -o vmc_mqtt vmc_mqtt.c -I/path/to/includes -L/path/to/libs -lpaho-mqtt3as -lpthread
```

## Usage

```bash
./vmc_mqtt [broker_address] [port]
```

* `broker_address`: (Optional) IP or hostname of the MQTT broker. Defaults to `127.0.0.1`.
* `port`: (Optional) MQTT port. Defaults to `1883`.

### Example

```bash
./vmc_mqtt 192.168.1.10 1883
```

## MQTT Topics

### Published Topics (examples)

* `vmc/speed` – Current fan speed level
* `vmc/NTC11`, `vmc/NTC12`, etc. – Temperature sensor readings
* `vmc/BP`, `vmc/S4`, `vmc/SLA`, etc. – Binary or analog sensor values

### Subscribed Topic

* `vmc/speed/set` – Accepts values `0`, `1`, `2`, or `3` to control VMC speed level

## Command Mapping

* `0` → OFF
* `1` → ON
* `2` → SPEED 1
* `3` → SPEED 2
* `4` → SPEED 3

## Signals & Exit Handling

* `SIGINT` / `SIGQUIT` handled gracefully to disconnect from the MQTT broker and free resources.

## Notes

* `DEV_Config` and hardware initialization functions must be provided or adapted for your specific environment.
* Serial port is hardcoded to `/dev/ttySC0` and expects specific binary protocols.
* Device credentials (username/password) are currently hardcoded. Secure this if deployed in production.

## License

This project is provided as-is. Adapt and reuse as needed.

---
