#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

const char *ssid = "IoT Scissor Lifter";  // Name of the Access Point
const char *password = "8860055120";      // Password for the Access Point

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket server on port 81

#define ENA_PIN 5           // PWM pin for motor speed control
#define IN1_PIN 18          // Motor direction control pin 1
#define IN2_PIN 19          // Motor direction control pin 2
#define UP_LIMIT_PIN 22     // Up limit switch pin
#define DOWN_LIMIT_PIN 23   // Down limit switch pin
#define UP_SWITCH_PIN 27    // Up switch pin
#define DOWN_SWITCH_PIN 26  // Down switch pin
#define RAMP_INTERVAL 20    // Time interval between speed increments (in milliseconds)
#define ONBOARD_LED 2       // Onboard LED pin

int speed = 190;             // Variable to store the motor speed
int direction;               // Variable to store the motor direction (0 - stop, 1 - up, 2 - down)
int currentSpeed = 50;       // Variable to store the current motor speed
bool upLimitSwitchStatus;    // Variable to indicate up limit switch state
bool downLimitSwitchStatus;  // Variable to indicate down limit switch state

void handleRoot() {
  Serial.println("Handling Root Request");
  server.send(200, "text/html", R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>FuturisticIOx +918860055120 Wifi Controlled Hydraulic Scissor Lifter</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f0f0f0;
      margin: 0;
      padding: 0;
    }

    .container {
      text-align: center;
      margin-top: 20px;
      background-color: #ffffff;
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      padding: 20px;
      max-width: 400px;
      margin: 0 auto;
    }

    h1 {
      font-size: 24px;
      color: #333333;
    }

    .button-selected {
      background-color: red !important; /* Change button color to red */
    }

    button {
      margin: 10px;
      padding: 15px 30px;
      font-size: 18px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
    }

    button:hover {
      background-color: #45a049;
    }

    input[type="range"] {
      width: 80%;
      margin-top: 10px;
      -webkit-appearance: none;
      appearance: none;
      background: #d3d3d3;
      height: 15px;
      border-radius: 5px;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    input[type="range"]:hover {
      opacity: 1;
    }

    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    input[type="range"]::-moz-range-thumb {
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: green;
      cursor: pointer;
    }

    .led {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      display: inline-block;
      margin: 10px;
      background-color: blue; /* Default color */
    }

    .led.on {
      background-color: red;
    }

    #status {
      font-size: 18px;
      margin-top: 20px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>FuturisticIOx +918860055120 Wifi Controlled Hydraulic Scissor Lifter</h1>
    <div>
      <div id="upLimitSwitch" class="led"></div>Up
      <span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Limit Switch &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>
      Dn<div id="downLimitSwitch" class="led"></div>
      
    </div>
    <br>
    <button id="upButton" onclick="selectButton('upButton'); moveUp()">Up</button>
    <button id="stopButton" onclick="selectButton('stopButton'); stop()">Stop</button>
    <button id="downButton" onclick="selectButton('downButton'); moveDown()">Down</button>
    <br><br>
    <label for="speed">Speed:</label>
    <input type="range" id="speed" min="50" max="255" value="200" onchange="changeSpeed(this.value)">
    <div id="status"></div>
  </div>

  <script>
    var socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    var selectedButton = null;

    socket.onopen = function(event) {
      console.log('WebSocket connected');
    };

    socket.onerror = function(error) {
      console.error('WebSocket error: ', error);
    };

    socket.onmessage = function(event) {
      var data = JSON.parse(event.data);
      document.getElementById("status").innerText = data.message;
      document.getElementById("speed").value = data.speed;
      document.getElementById("upLimitSwitch").className = data.upLimitStatus ? "led on" : "led off";
      document.getElementById("downLimitSwitch").className = data.downLimitStatus ? "led on" : "led off";
    };

    function changeSpeed(value) {
      sendData({ speed: value });
    }

    function selectButton(buttonId) {
      var buttons = document.querySelectorAll('button');
      buttons.forEach(function(btn) {
        if (btn.id === buttonId) {
          btn.classList.add('button-selected');
          selectedButton = buttonId;
        } else {
          btn.classList.remove('button-selected');
        }
      });
    }

    function moveUp() {
      sendData({ direction: 1 });
    }

    function moveDown() {
      sendData({ direction: 2 });
    }

    function stop() {
      sendData({ direction: 0 });
    }

    function sendData(data) {
      if (selectedButton) {
        socket.send(JSON.stringify(data));
      } else {
        alert('Please select a button.');
      }
    }
  </script>
</body>
</html>
)=====");
}

void handleUpdate() {
  Serial.println("Handling Update Request");
  StaticJsonDocument<200> doc;
  deserializeJson(doc, server.arg("plain"));

  int newSpeed = doc["speed"];
  direction = doc["direction"];

  if (newSpeed == 0) {
    // Handle speed command
  } else {
    speed = newSpeed;
    currentSpeed = 50;  // Reset current speed
  }

  // Prepare response JSON
  DynamicJsonDocument responseDoc(200);
  responseDoc["message"] = "Received data successfully";
  responseDoc["speed"] = speed;
  responseDoc["upLimitStatus"] = upLimitSwitchStatus;
  responseDoc["downLimitStatus"] = downLimitSwitchStatus;

  // Serialize response JSON and send it back via WebSocket
  String response;
  serializeJson(responseDoc, response);
  webSocket.broadcastTXT(response);
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_TEXT:
      {
        // Parse incoming JSON data
        StaticJsonDocument<200> doc;
        deserializeJson(doc, payload);

        int newSpeed = doc["speed"];
        direction = doc["direction"];

        if (newSpeed == 0) {
          // Handle speed command
        } else {
          speed = newSpeed;
          currentSpeed = 50;  // Reset current speed
        }

        // Prepare response JSON
        DynamicJsonDocument responseDoc(200);
        responseDoc["message"] = "Received data successfully";
        responseDoc["speed"] = speed;
        responseDoc["upLimitStatus"] = upLimitSwitchStatus;
        responseDoc["downLimitStatus"] = downLimitSwitchStatus;

        // Serialize response JSON and send it back via WebSocket
        String response;
        serializeJson(responseDoc, response);
        webSocket.sendTXT(num, response);
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(ONBOARD_LED, OUTPUT);  // Set onboard LED pin as output

  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(UP_LIMIT_PIN, INPUT_PULLUP);     // Set up limit switch pin as input with pull-up resistor
  pinMode(DOWN_LIMIT_PIN, INPUT_PULLUP);   // Set down limit switch pin as input with pull-up resistor
  pinMode(UP_SWITCH_PIN, INPUT_PULLUP);    // Set up switch pin as input with pull-up resistor
  pinMode(DOWN_SWITCH_PIN, INPUT_PULLUP);  // Set down switch pin as input with pull-up resistor


  //Wifi STA mode
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //   Serial.println("Connecting to WiFi...");
  // }
  // Serial.println("Connected to WiFi");

  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());  // Print IP address

  // Set up Access Point
  WiFi.softAP(ssid, password);

  Serial.println("Access Point started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize WebSocket server
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.begin();
}

void loop() {
  server.handleClient();
  webSocket.loop();  // Handling WebSocket events

  // Check and update the status of limit switches
  bool upLimitSwitchStatusNew = digitalRead(UP_LIMIT_PIN) == LOW;
  bool downLimitSwitchStatusNew = digitalRead(DOWN_LIMIT_PIN) == LOW;

  if (upLimitSwitchStatus != upLimitSwitchStatusNew || downLimitSwitchStatus != downLimitSwitchStatusNew) {
    // Update the status if there's a change
    upLimitSwitchStatus = upLimitSwitchStatusNew;
    downLimitSwitchStatus = downLimitSwitchStatusNew;

    // Prepare response JSON
    DynamicJsonDocument responseDoc(200);
    responseDoc["message"] = "Limit switch status updated";
    responseDoc["speed"] = speed;
    responseDoc["upLimitStatus"] = upLimitSwitchStatus;
    responseDoc["downLimitStatus"] = downLimitSwitchStatus;

    // Serialize response JSON and send it back via WebSocket
    String response;
    serializeJson(responseDoc, response);
    webSocket.broadcastTXT(response);
  }


  // Control motor based on direction and limit switch states
  if ((direction == 1 && digitalRead(UP_LIMIT_PIN) == LOW) || (digitalRead(UP_SWITCH_PIN) == LOW && digitalRead(UP_LIMIT_PIN) == LOW)) {
    stopMotor();  // Stop motor if up limit switch is triggered
  } else if ((direction == 2 && digitalRead(DOWN_LIMIT_PIN) == LOW) || (digitalRead(DOWN_SWITCH_PIN) == LOW && digitalRead(DOWN_LIMIT_PIN) == LOW)) {
    stopMotor();  // Stop motor if down limit switch is triggered
  } else {
    if (direction == 1) {
      moveUp();
    } else if (direction == 2) {
      moveDown();
    } else {
      if (digitalRead(UP_SWITCH_PIN) == LOW && direction == 0) {
        moveUp();
      } else if (digitalRead(DOWN_SWITCH_PIN) == LOW && direction == 0) {
        moveDown();
      } else if (digitalRead(UP_SWITCH_PIN) != LOW || digitalRead(DOWN_SWITCH_PIN) != LOW && direction == 0) {
        stopMotor();
      }
    }
  }
}


void moveUp() {
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  increaseSpeed();
}

void moveDown() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, HIGH);
  increaseSpeed();
}

void stopMotor() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
}

void increaseSpeed() {
  if (currentSpeed < speed) {
    currentSpeed++;
    analogWrite(ENA_PIN, currentSpeed);
    delay(RAMP_INTERVAL);
  }
}
