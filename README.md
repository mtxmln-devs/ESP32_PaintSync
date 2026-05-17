# 🎨 ESP32 Paint Sync

Control a WS2812B LED strip in real-time by painting on a canvas — directly from your browser. No app needed. Just connect, paint, and watch your LEDs light up.

![ESP32](https://img.shields.io/badge/ESP32-WiFi-blue?logo=espressif) ![FastLED](https://img.shields.io/badge/FastLED-WS2812B-orange) ![License](https://img.shields.io/badge/license-MIT-green)

---

## ✨ Features

- 🖌️ **Browser-based paint canvas** — draw from any phone or PC on the same network
- 🌈 **Real-time LED sync** — colors update as you draw (50ms throttle for stability)
- ↩️ **Undo support** — step back through up to 20 stroke history states
- 🗑️ **Clear canvas** — resets canvas and turns off LEDs
- 📏 **Adjustable brush size** — slider from 4px to 40px
- ⚫ **Black → Purple remap** — since black = LEDs off, black is auto-replaced with purple in the color picker
- 📱 **Mobile-friendly** — touch events supported, responsive layout

---

## 🛠️ Hardware Requirements

| Component | Details |
|---|---|
| Microcontroller | ESP32 (any variant) |
| LED Strip | WS2812B (default: 100 LEDs) |
| Data Pin | GPIO 13 |
| Power Supply | 5V, adequate amperage for your strip |

> **Power tip:** At full white brightness, WS2812B LEDs draw ~60mA each. For 100 LEDs, use at least a 6A 5V supply and power the strip directly — not through the ESP32.

---

## 📦 Dependencies

Install these libraries via the Arduino Library Manager:

| Library | Purpose |
|---|---|
| [FastLED](https://github.com/FastLED/FastLED) | LED strip control |
| `WiFi.h` | Built-in ESP32 WiFi |
| `WebServer.h` | Built-in ESP32 HTTP server |

---

## ⚙️ Configuration

Open the `.ino` file and edit the top section:

```cpp
// WiFi credentials
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// LED setup
#define DATA_PIN    13       // GPIO pin connected to LED strip DIN
#define NUM_LEDS    100      // Number of LEDs in your strip
#define LED_TYPE    WS2812B  // WS2811, SK6812, etc.
#define COLOR_ORDER GRB      // GRB for most WS2812B strips
```

---

## 🚀 Getting Started

### 1. Wire Your LED Strip

```
ESP32 GPIO 13  →  LED Strip DIN
ESP32 GND      →  LED Strip GND
5V Power Supply →  LED Strip VCC + GND
```

> Add a 300–500Ω resistor in series on the data line to protect against signal spikes.

### 2. Upload the Sketch

1. Open the `.ino` file in **Arduino IDE**
2. Select your ESP32 board under **Tools → Board**
3. Set the correct COM port
4. Click **Upload**

### 3. Find the IP Address

Open the **Serial Monitor** at `115200` baud. After connecting to WiFi, you'll see:

```
WiFi Connected!
IP Address: 192.168.x.x
```

### 4. Open the Web Interface

Navigate to `http://192.168.x.x` in any browser on the same network. Start painting — your LEDs will follow!

---

## 🖥️ Web Interface

| Control | Function |
|---|---|
| Color Picker | Choose the brush/LED color |
| Brush button | Active drawing mode |
| Undo | Restores previous stroke and LED color |
| Clear | Wipes canvas and turns off LEDs |
| Size Slider | Adjusts brush stroke width (4–40px) |

---

## 📡 API Endpoints

The ESP32 runs a lightweight HTTP server with two endpoints:

| Endpoint | Method | Description |
|---|---|---|
| `/` | GET | Serves the paint web app |
| `/set?hex=FF0000` | GET | Sets all LEDs to the given hex color |

**Special case:** Sending `#000000` (black) clears all LEDs (turns them off).

**Example:**
```
http://192.168.x.x/set?hex=00FF88
```

---

## 🧠 How It Works

```
User draws on canvas
        ↓
JavaScript picks the stroke color
        ↓
Fetch request → /set?hex=RRGGBB  (throttled to every 50ms)
        ↓
ESP32 parses hex → R, G, B bytes
        ↓
FastLED fill_solid() updates all LEDs
        ↓
FastLED.show() pushes data to strip
```

---

## 🐛 Troubleshooting

| Problem | Solution |
|---|---|
| LEDs flicker or show wrong colors | Check `COLOR_ORDER` — try `RGB` or `BGR` |
| Can't connect to WiFi | Verify SSID/password; ESP32 only supports 2.4GHz |
| Web page doesn't load | Confirm you're on the same network; check Serial Monitor for IP |
| LEDs too bright / too dim | Adjust `FastLED.setBrightness(100)` — range is 0–255 |
| Only first LED lights up | Check data pin wiring; add a pull-down resistor on DIN |

---

## 📁 Project Structure

```
esp32-paint-sync/
├── esp32_paint_sync.ino   # Main Arduino sketch
└── README.md              # This file
```

---

## 📜 License

MIT License — free to use, modify, and share.

---

## 🙌 Credits

Built with [FastLED](https://fastled.io/) and the ESP32 Arduino core. Inspired by the idea of making physical light reactive to digital creativity.
