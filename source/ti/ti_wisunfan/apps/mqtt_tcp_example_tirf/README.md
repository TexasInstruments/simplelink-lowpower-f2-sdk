# MQTT over Wi-SUN FAN — User Guide

## Architecture

```
                                  ┌─────────────────────────────────────────┐
                                  |      Border Router (Linux PC)           │
    BR: rcp_lmac                  |                                         │
   ┌─────────────┐    Spinel/UART |    ┌──────────────────────────────┐     │
   │   CC13xx    │◄───────────────|───►│        wfantund              │     │
   │    (RCP)    │                |    │   (Wi-SUN FAN daemon)        │     │
   └─────────────┘                |    └──────────────────────────────┘     │
          │                       |                 │                       │
          │ RF                    |                 │ TUN interface         │
          │                       |     ┌───────────▼──────────┐            │
          │                       |     │  mosquitto broker    │            │
          │                       |     │     port 5678        │            │
          │                       |     └──────────────────────┘            │
          |                       └─────────────────────────────────────────┘
          │
          │ Wi-SUN mesh (2020:abcd::/64)
          │
          ├──────────────────┬──────────────────┐
          │                  │                  │
   ┌──────▼───────┐   ┌──────▼───────┐   ┌──────▼───────┐
   │   CC13xx     │   │   CC13xx     │   │   CC13xx     │
   │ Router Node  │   │ Router Node  │   │ Router Node  │
   │    (RN 1)    │   │    (RN 2)    │   │    (RN n)    │
   │              │   │              │   │              │
   │ ns_node_mqtt │   | ns_node_mqtt │   |  ns_node_mqtt│
   │              │   │              │   │              │
   │              │   │              │   │              │
   │ BTN1→Publish │   │ BTN1→Publish │   │ BTN1→Publish │
   │              │   │              │   │              │
   └──────────────┘   └──────────────┘   └──────────────┘
```

### Component Roles

| Component | Description |
|---|---|
| **CC13xx RCP** | Radio Co-Processor — handles the Wi-SUN PHY/MAC layer. Connected to the Linux host via USB/UART. |
| **wfantund** | Linux daemon that manages the Wi-SUN FAN network. Forms the border router, assigns IPv6 prefix `2020:abcd::/64`, provides routing for mesh nodes. |
| **mosquitto** | MQTT broker running on the Linux host. Listens on port 5678. Receives and distributes MQTT messages between all connected nodes. |
| **Router Nodes** | CC13xx embedded devices flashed with `ns_node_mqtt`. Join the Wi-SUN mesh, connect to the broker via TCP, publish LED state on button press, and control their LED based on received messages. |

### Network Flow

1. Router nodes join the Wi-SUN mesh managed by wfantund
2. Each node gets a unique IPv6 address: `2020:abcd::XX:XX:XX:XX`
3. The broker is reachable at the border router's mesh address: `2020:abcd::`
4. Nodes connect to `2020:abcd::` port 5678 over TCP through the Wi-SUN mesh
5. MQTT messages flow: Node → TCP → Wi-SUN mesh → Border Router → mosquitto → all subscribers

---

## Hardware Requirements

| Item | Quantity | Notes |
|---|---|---|
| CC13xx LaunchPad (RCP) | 1 | Connected to Linux PC via USB |
| CC13xx LaunchPad (Router Node) | 2 | Flashed with mqtt_tcp_example_tirf |
| Linux PC | 1 | Running wfantund + mosquitto |

---

## Software Requirements

- **wfantund** — Wi-SUN FAN daemon (from `lprf-dallas-wisun-wfantund`)
- **mosquitto** — MQTT broker (pre-installed and pre-configured on the border router)
- **mosquitto-clients** — for monitoring MQTT traffic (`mosquitto_sub`)
- **Uniflash** or **CCS** — for flashing firmware to CC13xx devices
- **wfanctl** — command-line tool included with wfantund

---

## Step 1 — Flash the Firmware

### 1.1 Flash the RCP firmware onto the border router CC13xx

Flash the RCP firmware (`rcp_lmac`) onto the CC13xx that will be connected to the Linux PC. Refer to the SDK documentation for the RCP build and flash procedure.

### 1.2 Flash `ns_node_mqtt` onto both router nodes

Build and flash `ns_node_mqtt` for your board:

1. Open CCS and import the project from:
   ```
   examples-dev/rtos/<BOARD>/ti_wisunfan/ns_node_mqtt_fullsrc/
   ```
2. Build with the **Release** configuration
3. Flash to both CC13xx router node devices

---

## Step 2 — Build the Docker Environment

The border router runs inside Docker containers. All commands below are run from the `lprf-dallas-wisun-wfantund` repository root.

### Build the containers

```bash
docker compose build
```

---

## Step 3 — Attach the RCP Device via USB

The CC13xx RCP must be forwarded from Windows into WSL so the Docker container can access it.

**In PowerShell (Windows):**

```powershell
# List all USB devices and find your CC13xx
usbipd list

# Bind the device (replace 2-6 with your actual bus ID from the list)
usbipd bind --busid 2-6

# Attach it to WSL
usbipd attach --wsl --busid 2-6
```

**In WSL (Ubuntu):**

```bash
# Verify the device is visible
ls -l /dev/ttyACM*
```

You should see a `/dev/ttyACM*` entry for the CC13xx.

---

## Step 4 — Start the Docker Containers

```bash
docker compose up -d
```

Verify all containers are running:

```bash
docker ps
```

You should see containers for `linux_host`, `wfantund`.

---

## Step 5 — Build and Run the Border Router (linux_host container)

Enter the `linux_host` container:

```bash
cd linux-host
docker compose exec -it linux_host bash
```

Confirm the RCP device is accessible inside the container:

```bash
ls -l /dev/ttyACM*
```

### 5.1 Build the project *(first time only)*

> These two commands only need to be run once. On subsequent runs, skip straight to Step 5.2.

Generate the build files:

```bash
cmake -G Ninja .
```

Build the project:

```bash
ninja
```

### 5.2 Run the border router

Start the border router binary (this connects to the RCP and starts the Wi-SUN network):

```bash
./bin/wisun-rcp-host-br apps/border_router_nanostack_tirf/border_router_host.cfg
```

> **Leave this terminal running.** The border router must stay active throughout the demo.

---

## Step 6 — Build and Start wfantund (wfantund container)

Open a **new terminal** and enter the `wfantund` container:

```bash
cd <path-to-lprf-dallas-wisun-wfantund>
docker compose exec -it wfantund bash
```

Build and install wfantund inside the container:

```bash
./bootstrap.sh && ./configure --sysconfdir=/etc && make install
```

Start the D-Bus service and verify the installation:

```bash
service dbus start
wfantund --version
```

Start wfantund and connect it to the border router running in `linux_host`:

```bash
wfantund -o Config:NCP:SocketPath tcp:linux_host:4902 -o IPv6:WfantundGlobalAddress 2020:ABCD::/64
```

> **Leave this terminal running.** wfantund manages the Wi-SUN network and must stay active.

---

## Step 7 — Control the Network with wfanctl

Open another **new terminal** and enter the `wfantund` container:

```bash
cd <path-to-lprf-dallas-wisun-wfantund>
docker compose exec -it wfantund bash
```

Start wfanctl:

```bash
wfanctl
```

Useful commands inside wfanctl:

```
status                      # show current network state
set interface:up true       # bring up the Wi-SUN interface and start the network
get panid                   # show the PAN ID
get networkname             # show the network name
get connecteddevices        # list all nodes currently connected to the mesh
set interface:up false      # bring down the interface (leave network)
```

Once `set interface:up true` is run, the border router begins advertising the Wi-SUN network and router nodes can join.

---

## Step 8 — Start the MQTT Broker (wfantund container)

mosquitto is pre-installed and pre-configured inside the `wfantund` container. Open another **new terminal**, enter the container, and start the broker:

```bash
cd <path-to-lprf-dallas-wisun-wfantund>
docker compose exec -it wfantund bash
```

```bash
sudo mosquitto -c /etc/mosquitto/mosquitto.conf -v
```

Expected output:
```
mosquitto version X.X.X starting
Config loaded from /etc/mosquitto/mosquitto.conf
Opening ipv6 listen socket on port 5678
mosquitto version X.X.X running
```

> **Leave this terminal open.** Broker logs showing node connections and messages will appear here.

---

## Step 9 — Power On the Router Nodes

1. Power on both CC13xx router node devices (already flashed with `ns_node_mqtt`)
2. The **Red LED (RLED)** will blink while the node is searching for and joining the Wi-SUN network
3. Once joined, the **Green LED (GLED)** turns solid ON and RLED turns OFF

The join process typically takes **30–90 seconds**. Verify the nodes have joined using wfanctl:

```bash
get connecteddevices
```

---

## Step 10 — Verify MQTT Connections

### 10.1 Watch mosquitto logs

Check the mosquitto terminal (Step 8). When each node connects and subscribes you should see:

```
New connection from 2020:abcd::212:4b00:XXXX:XXXX on port 5678
Client 02124b002d1628ef connected
Client 02124b002d1628ef subscribed to wisun/status
```

Each node uses a client ID derived from its MAC address.

### 10.2 Monitor MQTT traffic with mosquitto_sub

Open another terminal, enter the `wfantund` container, and subscribe to the topic:

```bash
docker compose exec -it wfantund bash
mosquitto_sub -h localhost -p 5678 -t "wisun/#" -v 
```

---

## Step 11 — Demonstrate the Example

### Publish: Button Controls LED and Broadcasts State

BTN1 triggers two publishes — one on press and one on release:

| BTN1 action | Published payload |
|---|---|
| **Press** | `{"device":"...","led":1}` |
| **Release** | `{"device":"...","led":0}` |

The node publishes the new state to `wisun/status`. mosquitto distributes the message to all other subscribers.

### Subscribe: Receive → Mirror LED

When a node receives a PUBLISH on `wisun/status` it reads the `"led"` field and applies the same state to its own RLED:

- Receives `"led":1` → RLED turns **ON**
- Receives `"led":0` → RLED turns **OFF**

The receiver's LED exactly mirrors what the sender's LED is doing in real time — while you hold BTN1 on Node 1, both Node 1 and Node 2 have their RLED on; when you release, both turn off.

### Expected Demo Sequence

```
[Node 1]  BTN1 pressed
[Node 1]  Publishes: {"device":"...ef","led":1}
[Broker]  Distributes to all subscribers
[Node 2]  Receives PUBLISH → RLED turns ON

[Node 1]  BTN1 releasedF
[Node 1]  Publishes: {"device":"...ef","led":0}
[Broker]  Distributes to all subscribers
[Node 2]  Receives PUBLISH → RLED turns OFF
```

### Connection monitoring

The firmware uses a 30-second connect timeout. If no CONNACK arrives within 30 seconds, the connection is cleaned up and `mqtt_state` returns to `MQTT_STATE_IDLE`. The main loop also polls the connection state — if the broker closes the connection, the node disconnects cleanly.

---

