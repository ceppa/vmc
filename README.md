# MQTT UART Client

A lightweight MQTT client in C that reads data from UART, processes it, and publishes to an MQTT broker.  
Supports reading MQTT credentials from a config file with automatic creation of default config.

---

## Features

- Connects to an MQTT broker using username and password read from a config file (`mqtt.cfg`).
- If `mqtt.cfg` does not exist, it creates the file with default placeholders and exits with instructions.
- Reads and parses UART data, publishes updates to configured MQTT topics.
- Handles reconnection, subscription, and graceful shutdown.
- Designed for embedded Linux systems with UART support.

---

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

---

## Build & Run

### Prerequisites

- GCC compiler
- [Paho MQTT Async C Client Library](https://www.eclipse.org/paho/clients/c/) installed
- Development headers and libraries for UART (depends on platform)
- `DEV_Config.h` and UART helper files (specific to your platform)

### Compile

```bash
gcc -o mqtt_uart_client main.c -lpaho-mqtt3as -lrt -pthread
````

### Run

```bash
./mqtt_uart_client [broker_ip] [broker_port]
```

* Defaults to `127.0.0.1` and port `1883` if arguments are omitted.

---

## Usage

1. Run the client once to generate the default config file:

   ```bash
   ./mqtt_uart_client
   ```

   You will see a message asking to update `mqtt.cfg`.

2. Edit `mqtt.cfg` and set your MQTT broker username and password.

3. Run the client again:

   ```bash
   ./mqtt_uart_client your.broker.address 1883
   ```

---

## Signal Handling

* Supports `SIGINT` (Ctrl+C) and `SIGQUIT` for graceful exit.
* Catches segmentation faults and logs an error message.

---

## License

This project is licensed under the GNU General Public License v3.0 (GPL-3.0).  
See the [LICENSE](LICENSE) file for details.

---

## Author

Carlo Ceppa — [carlo.ceppa@gmail.com](mailto:carlo.ceppa@gmail.com)

---
