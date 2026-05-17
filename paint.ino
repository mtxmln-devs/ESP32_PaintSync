#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>

// --- Configuration ---
const char* ssid = "POCO X7 Pro";
const char* password = "laurence";

#define DATA_PIN 13       // Pin connected to your LED strip
#define NUM_LEDS 100       // Number of LEDs in your strip
#define LED_TYPE WS2812B  // Change if using WS2811, SK6812, etc.
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
WebServer server(80);

// HTML & JavaScript
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Paint Sync</title>
<style>
    body{ margin:0; padding:0; background:#111; color:white; font-family:sans-serif; display:flex; flex-direction:column; align-items:center; overflow:hidden; }
    h1{ font-size:22px; margin:15px 0 10px; }
    .toolbar{ width:95%; max-width:420px; display:flex; flex-wrap:wrap; gap:10px; justify-content:center; align-items:center; margin-bottom:10px; }
    button{ padding:10px 16px; border:none; border-radius:10px; background:#222; color:white; font-size:14px; cursor:pointer; }
    button:active{ background:#444; }
    #brushBtn.active{ background:#00aaff; }
    input[type=color]{ width:60px; height:45px; border:none; background:none; cursor:pointer; }
    canvas{ background:white; width:95vw; max-width:420px; height:95vw; max-height:420px; border:3px solid #333; border-radius:14px; touch-action:none; }
    #status{ margin-top:12px; color:#00ff88; font-size:14px; }
</style>
</head>
<body>

<h1>ESP32 Paint Sync</h1>

<div class="toolbar">
    <input type="color" id="colorPicker" value="#ff0000">
    <button id="brushBtn" class="active">Brush</button>
    <button id="undoBtn">Undo</button>
    <button id="clearBtn">Clear</button>
    <label> Size <input type="range" id="sizeSlider" min="4" max="40" value="12"> </label>
</div>

<canvas id="canvas"></canvas>
<div id="status">Ready</div>

<script>
    const canvas = document.getElementById('canvas');
    const ctx = canvas.getContext('2d', { willReadFrequently: true });
    const picker = document.getElementById('colorPicker');
    picker.addEventListener('input', ()=>{
    if(picker.value.toLowerCase() === '#000000'){

        picker.value = '#8a2be2';

        status.innerText =
            'Black replaced with purple';
    }
});

    const undoBtn = document.getElementById('undoBtn');
    const clearBtn = document.getElementById('clearBtn');
    const sizeSlider = document.getElementById('sizeSlider');
    const status = document.getElementById('status');

    canvas.width = 420;
    canvas.height = 420;
    ctx.fillStyle = '#d8d8d8';
    ctx.fillRect(0,0,canvas.width,canvas.height);

    let drawing = false;
    let history = [];      
    let colorHistory = []; 
    let lastSentTime = 0;
    const requestInterval = 50; // Minimum 50ms between drawing updates

    function sendColor(hex, force = false){
        const now = Date.now();
        // Only send if it's been long enough, OR if we are forcing it (like Undo)
        if (force || (now - lastSentTime > requestInterval)) {
            lastSentTime = now;
            fetch(`/set?hex=${hex.substring(1)}`)
            .then(() => { status.innerText = 'LED Synced: ' + hex; })
            .catch(() => { status.innerText = 'Sync Error'; });
        }
    }

    function getPos(e){
        const rect = canvas.getBoundingClientRect();
        let x, y;
        if(e.touches){
            x = e.touches[0].clientX - rect.left;
            y = e.touches[0].clientY - rect.top;
        } else {
            x = e.offsetX;
            y = e.offsetY;
        }
        return {x, y};
    }

    function startDraw(e){
        // Save history BEFORE starting the new stroke



        drawing = true;
        draw(e);
    }



undoBtn.onclick = () => {

    if(history.length > 0){

        // Remove latest stroke state
        history.pop();

        // Remove latest color
        colorHistory.pop();

        if(history.length > 0){

            // Restore previous canvas
            const previousImage =
                history[history.length - 1];

            ctx.putImageData(previousImage, 0, 0);

            // Restore PREVIOUS color
            const previousColor =
                colorHistory[colorHistory.length - 1];

            picker.value = previousColor;

            sendColor(previousColor, true);

            status.innerText =
                'Restored: ' + previousColor;

        }else{

            // Empty canvas
            ctx.fillStyle = '#d8d8d8';

            ctx.fillRect(
                0,
                0,
                canvas.width,
                canvas.height
            );

            sendColor('#000000', true);

            status.innerText =
                'Canvas empty';
        }
    }
};

function stopDraw(){

    history.push(
        ctx.getImageData(
            0,
            0,
            canvas.width,
            canvas.height
        )
    );

    colorHistory.push(picker.value);

    if(history.length > 20){

        history.shift();
        colorHistory.shift();
    }

    drawing = false;

    ctx.beginPath();

    // Send final color once drawing ends
    sendColor(picker.value, true);
}

function draw(e){

    if(!drawing) return;

    e.preventDefault();

    const pos = getPos(e);

    ctx.lineWidth = sizeSlider.value;

    ctx.lineCap = 'round';

    ctx.strokeStyle = picker.value;

    ctx.lineTo(pos.x, pos.y);

    ctx.stroke();

    ctx.beginPath();

    ctx.moveTo(pos.x, pos.y);

    // Send color but respect interval limit
    sendColor(picker.value, false);
}

clearBtn.onclick = () => {

    ctx.fillStyle = '#d8d8d8';

    ctx.fillRect(
        0,
        0,
        canvas.width,
        canvas.height
    );

    history = [];
    colorHistory = [];

    sendColor('#000000', true);

    status.innerText =
        'Canvas cleared';
};

    canvas.addEventListener('mousedown', startDraw);
    canvas.addEventListener('mouseup', stopDraw);
    canvas.addEventListener('mousemove', draw);

    canvas.addEventListener('touchstart', (e)=>{
        e.preventDefault();
        startDraw(e);
    });

    canvas.addEventListener('touchend', (e)=>{
        e.preventDefault();
        stopDraw(e);
    });

    canvas.addEventListener('touchmove', (e)=>{
        e.preventDefault();
        draw(e);
    });
</script>


</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleSetColor() {
  if (server.hasArg("hex")) {
    String hexStr = server.arg("hex");
    long number = strtol(hexStr.c_str(), NULL, 16);

    byte r = number >> 16;
    byte g = (number >> 8) & 0xFF;
    byte b = number & 0xFF;

  if(r == 0 && g == 0 && b == 0){

    FastLED.clear();
    FastLED.show();

    server.send(200, "text/plain", "OFF");

    return;
}

    fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
    FastLED.show();
    server.send(200, "text/plain", "OK");
  }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(100);  // Set a default brightness

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/set", handleSetColor);
  server.begin();
}

void loop() {
  server.handleClient();
}
