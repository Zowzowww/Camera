#ifndef CAMERA_INDEX_H_
#define CAMERA_INDEX_H_

const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-CAM Oil Detection</title>
    <script>
        function updateDetection() {
            fetch('/detect')
            .then(response => response.json())
            .then(data => {
                document.getElementById("goodOilCount").innerText = data.good_oil;
                document.getElementById("badOilCount").innerText = data.bad_oil;
            });
        }

        setInterval(updateDetection, 2000);
    </script>
</head>
<body>
    <h1>ESP32-CAM Oil Detection</h1>

    <h2>Oil Detection Results</h2>
    <p>🟡 Good Oil Count: <span id="goodOilCount">0</span></p>
    <p>⚫ Bad Oil Count: <span id="badOilCount">0</span></p>

    <img id="stream" src="/stream" width="100%">

    <br><br>
    <button onclick="location.href='/secret'">➡️ Go to Secret Settings</button>
</body>
</html>
)rawliteral";

const char PROGMEM SECRET_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Oil Detection Control</title>
    <script>
        function enableGoodOilDetection() {
            fetch('/setoil?oil=good')
            .then(() => {
                console.log("Good Oil detected!");
                let count = parseInt(document.getElementById("goodOilCount").innerText);
                document.getElementById("goodOilCount").innerText = count + 1;
            });
        }

        function enableBadOilDetection() {
            fetch('/setoil?oil=bad')
            .then(() => {
                console.log("Bad Oil detected!");
                let count = parseInt(document.getElementById("badOilCount").innerText);
                document.getElementById("badOilCount").innerText = count + 1;
            });
        }

        function disableDetection() {
            fetch('/setoil?oil=none');
        }
    </script>
</head>
<body>
    <h1>Oil Detection Control</h1>

    <h2>Oil Detection Results</h2>
    <p>🟡 Good Oil Count: <span id="goodOilCount">0</span></p>
    <p>⚫ Bad Oil Count: <span id="badOilCount">0</span></p>

    <button onclick="enableGoodOilDetection()">🟡 Enable Good Oil Detection</button>
    <button onclick="enableBadOilDetection()">⚫ Enable Bad Oil Detection</button>
    <button onclick="disableDetection()">❌ Disable Detection</button>

    <h2>Live Camera Feed</h2>
    <img id="stream" src="/stream" width="100%">

    <br><br>
    <button onclick="location.href='/'">⬅️ Back to Live Stream</button>
</body>
</html>
)rawliteral";

#endif /* CAMERA_INDEX_H_ */
