import csv
import os
import time
from collections import deque

import numpy as np
import serial
from serial.tools import list_ports
import matplotlib.pyplot as plt

PORT = None        # None = auto-detect, or set e.g. 'COM5' or '/dev/ttyACM0'
BAUD = 115200
HISTORY = 60       # frames kept in the heatmap
LOGFILE = 'nrf_log.csv'


def find_port():
    ports = list(list_ports.comports())
    for p in ports:
        if p.vid == 0x2341:          # Arduino vendor id
            return p.device
    return ports[0].device if ports else None


port = PORT or find_port()
if port is None:
    raise SystemExit('No serial port found. Plug in the Nano and close the serial monitor.')
print('Using', port)

ser = serial.Serial(port, BAUD, timeout=3)
new_file = not os.path.exists(LOGFILE) or os.path.getsize(LOGFILE) == 0
log = open(LOGFILE, 'a', newline='')
writer = csv.writer(log)
if new_file:
    writer.writerow(['time', 'alert', 'reason', 'sweeps'] + [f'ch{c}' for c in range(126)])

plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 7))
freqs = np.arange(2400, 2526)
history = deque(maxlen=HISTORY)


def draw(ratio, alert, reason):
    ax1.clear()
    ax1.bar(freqs, ratio, width=1.0, color='red' if alert else 'tab:green')
    ax1.set_xlim(2399, 2526)
    ax1.set_ylim(0, 1.1)
    for name, f in (('Wi-Fi 1', 2412), ('Wi-Fi 6', 2437), ('Wi-Fi 11', 2462)):
        ax1.axvline(f, color='gray', linestyle=':')
        ax1.text(f, 1.03, name, ha='center', fontsize=8)
    ax1.set_ylabel('Busy share of sweeps')
    ax1.set_xlabel('Frequency (MHz)')
    ax1.set_title('2.4 GHz activity: ' + reason)

    ax2.clear()
    ax2.imshow(np.array(history).T, aspect='auto', origin='lower',
               extent=[-len(history), 0, 2400, 2525], cmap='viridis', vmin=0, vmax=1)
    ax2.set_xlabel('Frames ago')
    ax2.set_ylabel('Frequency (MHz)')
    plt.tight_layout()
    plt.pause(0.01)


try:
    while plt.fignum_exists(fig.number):
        line = ser.readline().decode(errors='ignore').strip()
        if not line.startswith('SWEEP,'):
            plt.pause(0.01)          # keeps the window responsive
            continue
        try:
            parts = line.split(',')
            alert = parts[2] == '1'
            sweeps = int(parts[3])
            reason = parts[4]
            hits = [int(x) for x in parts[5:]]
        except (ValueError, IndexError):
            continue
        if len(hits) != 126:
            continue                 # half-received line

        writer.writerow([time.strftime('%H:%M:%S'), int(alert), reason, sweeps] + hits)
        log.flush()
        ratio = np.array(hits) / sweeps
        history.append(ratio)
        draw(ratio, alert, reason)
finally:
    ser.close()
    log.close()