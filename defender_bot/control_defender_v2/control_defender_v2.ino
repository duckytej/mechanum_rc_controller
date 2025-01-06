#include <WiFi.h>
#include <WebServer.h>

// Motor pins
#define FLP 33  // Front Left Positive
#define FLN 32  // Front Left Negative
#define FRP 25  // Front Right Positive
#define FRN 26  // Front Right Negative

// Network credentials
const char* ssid = "Cheese_bot";
const char* password = "soccerrobo";

const unsigned int commandInterval = 25;
unsigned long lastCommandTime = 0;
unsigned long currMilis = 0;

WebServer server(80);

// Structure to hold motor states
struct MotorStates {
  int RState;
  int LState;
} myData = {0, 0};

void setup() {
  Serial.begin(115200);
  
  // Motor pin setup
  pinMode(FLP, OUTPUT);
  pinMode(FLN, OUTPUT);
  pinMode(FRP, OUTPUT);
  pinMode(FRN, OUTPUT);

  // Create AP
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/control", HTTP_POST, handleControl);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  currMilis = millis();
  if (currMilis - lastCommandTime > commandInterval) {
    commands();
    lastCommandTime = millis();
  }
}

void handleRoot() {
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name='viewport' content='width=device-width, initial-scale=1'>
      <style>
        body {
          margin: 0;
          padding: 10px;
          display: flex;
          justify-content: space-between;
          align-items: center;
          min-height: 100vh;
          background: #f0f0f0;
          font-family: Arial, sans-serif;
        }
        .container {
          display: flex;
          width: 100%;
          justify-content: space-between;
        }
        .column {
          display: flex;
          flex-direction: column;
          gap: 10px;
        }
        .left-controls {
          display: flex;
          gap: 10px;
          margin-left: 10px;
        }
        .right-controls {
          display: flex;
          gap: 10px;
          margin-right: 10px;
        }
        button {
          width: 120px;
          height: 80px;
          border: none;
          border-radius: 10px;
          background: #007bff;
          color: white;
          font-size: 16px;
          cursor: pointer;
          transition: background 0.3s;
          padding: 10px;
          text-align: center;
        }
        button:active {
          background: #0056b3;
        }
      </style>
    </head>
    <body>
      <div class='container'>
        <div class='left-controls'>
          <div class='column'>
            <button onmousedown='control("L", 255)' onmouseup='control("L", 0)' ontouchstart='control("L", 255)' ontouchend='control("L", 0)'>Left Full Forward</button>
            <button onmousedown='control("L", -255)' onmouseup='control("L", 0)' ontouchstart='control("L", -255)' ontouchend='control("L", 0)'>Left Full Back</button>
          </div>
          <div class='column'>
            <button onmousedown='control("L", 128)' onmouseup='control("L", 0)' ontouchstart='control("L", 128)' ontouchend='control("L", 0)'>Left Half Forward</button>
            <button onmousedown='control("L", -128)' onmouseup='control("L", 0)' ontouchstart='control("L", -128)' ontouchend='control("L", 0)'>Left Half Back</button>
          </div>
        </div>
        <div class='right-controls'>
          <div class='column'>
            <button onmousedown='control("R", 128)' onmouseup='control("R", 0)' ontouchstart='control("R", 128)' ontouchend='control("R", 0)'>Right Half Forward</button>
            <button onmousedown='control("R", -128)' onmouseup='control("R", 0)' ontouchstart='control("R", -128)' ontouchend='control("R", 0)'>Right Half Back</button>
          </div>
          <div class='column'>
            <button onmousedown='control("R", 255)' onmouseup='control("R", 0)' ontouchstart='control("R", 255)' ontouchend='control("R", 0)'>Right Full Forward</button>
            <button onmousedown='control("R", -255)' onmouseup='control("R", 0)' ontouchstart='control("R", -255)' ontouchend='control("R", 0)'>Right Full Back</button>
          </div>
        </div>
      </div>
      <script>
        function control(side, value) {
          fetch('/control', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: `side=${side}&value=${value}`
          });
        }
      </script>
    </body>
    </html>
  )";
  server.send(200, "text/html", html);
}

void handleControl() {
  if (server.hasArg("side") && server.hasArg("value")) {
    String side = server.arg("side");
    int value = server.arg("value").toInt();
    
    if (side == "L") {
      myData.LState = value;
    } else if (side == "R") {
      myData.RState = value;
    }
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void commands() {
  int R = constrain(myData.RState, -255, 255);
  int L = constrain(myData.LState, -255, 255);

  // Right side motor control
  if (R > 10) {
    MRF();
  } else if (R < -10) {
    MRB();
  } else {
    MRS();
  }

  // Left side motor control
  if (L > 10) {
    MLF();
  } else if (L < -10) {
    MLB();
  } else {
    MLS();
  }

  Serial.print("L: ");
  Serial.print(L);
  Serial.print(" R: ");
  Serial.println(R);
}

void MRF() {
  digitalWrite(FRP, HIGH);
  digitalWrite(FRN, LOW);
}

void MRB() {
  digitalWrite(FRP, LOW);
  digitalWrite(FRN, HIGH);
}

void MRS() {
  digitalWrite(FRP, LOW);
  digitalWrite(FRN, LOW);
}

void MLF() {
  digitalWrite(FLP, HIGH);
  digitalWrite(FLN, LOW);
}

void MLB() {
  digitalWrite(FLP, LOW);
  digitalWrite(FLN, HIGH);
}

void MLS() {
  digitalWrite(FLP, LOW);
  digitalWrite(FLN, LOW);
}
