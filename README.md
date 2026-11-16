# RF Analyzer Model

A low cost 2.4 GHz spectrum analyzer built with an Arduino Nano ESP32 and an nRF24L01+ PA+LNA module. It scans the 2.4 GHz band, shows activity on a small OLED and streams the data to a PC for a live plot and CSV logging.

## Features

- Scans the 2.4 GHz band channel by channel using the nRF24L01+ received power detector
- Live bar display on a 0.96 inch I2C SSD1306 OLED
- LED on D6 as an activity indicator
- Python script on the PC for a real time spectrum plot
- Automatic CSV logging of scan data for later analysis

## Hardware

| Component | Purpose |
|---|---|
| Arduino Nano ESP32 | Main microcontroller |
| nRF24L01+ PA+LNA | 2.4 GHz receiver used for scanning |
| 0.96" I2C SSD1306 OLED | On device spectrum display |
| LED (D6) | Activity indicator |

## Wiring

Pin assignments are defined at the top of `src/main.cpp`. Match your wiring to those definitions.

| nRF24L01+ | Nano ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| CE, CSN, SCK, MOSI, MISO | See `src/main.cpp` |

| SSD1306 OLED | Nano ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA, SCL | I2C pins (see `src/main.cpp`) |

A 10 uF capacitor across the nRF24L01+ power pins is recommended. The PA+LNA module is sensitive to power noise and can give unstable readings without it.

## Project structure

```
RF_Analyzer_Model/
├── src/            Firmware (main.cpp)
├── include/        Header files
├── lib/            Project libraries
├── pc/             Python live plot script (live_plot.py)
├── test/           Tests
└── platformio.ini  PlatformIO configuration
```

## Getting started

### Firmware

1. Install [VS Code](https://code.visualstudio.com/) and the [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
2. Clone the repo

   ```bash
   git clone https://github.com/Yato-Kami-23/RF_Analyzer_Model.git
   ```

3. Open the folder in VS Code and let PlatformIO install the dependencies
4. Connect the Nano ESP32 over USB and click **Upload**

### PC live plot

1. Install the Python dependencies

   ```bash
   pip install pyserial matplotlib
   ```

2. Set the correct COM port inside `live_plot.py`
3. Run the script

   ```bash
   python pc/live_plot.py
   ```

The plot updates in real time and each scan is saved to `nrf_log.csv`.

## How it works

The nRF24L01+ cannot measure signal strength directly. For each of the 126 channels (2.400 to 2.525 GHz) the firmware tunes the radio to that channel, listens briefly and checks the received power detector flag. Repeating this many times per channel builds a hit count that reflects how busy each part of the band is. The counts are drawn on the OLED and sent over serial to the PC.

## Sample output

Add a screenshot of the live plot and a photo of the build here.

## Future improvements

- Peak hold and averaging modes
- Wi-Fi channel overlay for 2.4 GHz networks
- Wireless streaming of scan data from the ESP32 to the PC

## Author

Yato ([@Yato-Kami-23](https://github.com/Yato-Kami-23))
