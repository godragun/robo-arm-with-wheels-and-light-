
#include <Arduino.h>
#include "esp32-hal-ledc.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <math.h>
#include "index_html.h"
#include "three_js_asset.h"
#include "orbitcontrols_js_asset.h"

// Motor pins
#define IN1 14
#define IN2 17
#define IN3 18
#define IN4 19
#define ENA 13
#define ENB 12

// Built-in LED pin (GPIO 2 on most ESP32 boards)
#define LED_BUILTIN 2

// Relay/Light pin (GPIO 4)
#define LIGHT_PIN 4

// LEDC PWM (ESP32) for motor enable pins
const int PWM_CHANNEL_A = 0;
const int PWM_CHANNEL_B = 1;
const int PWM_FREQ = 1000;   // 1 kHz
const int PWM_RES = 8;       // 8-bit (0-255)

// Servo PWM driver
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40); // Default I2C address
#define SERVO_MIN 150  // Pulse length for 0°
#define SERVO_MAX 600  // Pulse length for 180°

// Servo stability settings
#define SERVO_STABILIZE_DELAY 50   // Additional delay after reaching target (ms) for stability

// Store current servo positions (in mapped angle 0-180)
float currentServoAngles[6] = {0, 0, 0, 0, 0, 0};

// Servo smoothing and vibration reduction settings
const float SERVO_DEADBAND = 2.0f;        // Ignore changes smaller than ±2°
const unsigned long SERVO_MIN_UPDATE_MS = 50;  // Minimum 50ms between servo updates
unsigned long lastServoUpdate[6] = {0, 0, 0, 0, 0, 0};  // Track last update time per servo
float targetServoAngles[6] = {0, 0, 0, 0, 0, 0};  // Target angles for smooth easing
const float SERVO_EASING_ALPHA = 0.15f;   // Easing factor (0.0 = very slow, 1.0 = instant)

// SoftAP configuration (open network as requested)
const char* ap_ssid = "robo arm";   // Visible SSID
const char* ap_password = "";       // Empty password -> open network
IPAddress ap_ip(192, 168, 4, 1);
IPAddress ap_gateway(192, 168, 4, 1);
IPAddress ap_subnet(255, 255, 255, 0);

// PID tuning values for smoother servo motions (reduced gains to minimize vibration)
const float PID_KP = 0.5f;              // Reduced from 0.65
const float PID_KI = 0.01f;             // Reduced from 0.04 (integral windup prevention)
const float PID_KD = 0.12f;             // Reduced from 0.18
const float PID_MAX_INTEGRAL = 50.0f;   // Reduced from 120.0 (anti-windup)
const float PID_POSITION_TOLERANCE = 0.5f; // Increased tolerance
const float PID_MAX_STEP = 3.0f;        // Reduced from 5.0 (smaller steps)
const int   PID_MAX_ITERATIONS = 80;    // Reduced iterations
const int   SERVO_PID_STEP_DELAY = 20;  // Increased delay for smoother motion

struct PIDController {
  float kp;
  float ki;
  float kd;
  float integral;
  float lastError;
  unsigned long lastTime;
};

PIDController servoPid[6];

void resetPidController(int index) {
  if (index < 0 || index >= 6) return;
  servoPid[index].kp = PID_KP;
  servoPid[index].ki = PID_KI;
  servoPid[index].kd = PID_KD;
  servoPid[index].integral = 0.0f;
  servoPid[index].lastError = 0.0f;
  servoPid[index].lastTime = millis();
}

void initializePidControllers() {
  for (int i = 0; i < 6; i++) {
    resetPidController(i);
  }
}

// Web server
WebServer server(80);

void handleThreeJS() {
  server.send_P(200, "application/javascript", THREE_MIN_JS, THREE_MIN_JS_LEN);
}

void handleOrbitControlsJS() {
  server.send_P(200, "application/javascript", ORBIT_CONTROLS_JS, ORBIT_CONTROLS_JS_LEN);
}

// HTML content with dynamic ESP32 IP (from PROGMEM header)
String getHtmlPage() {
    String esp32IP = WiFi.softAPIP().toString();
    // Check if HTML is loaded
    if (strlen(INDEX_HTML) == 0) {
      Serial.println("ERROR: INDEX_HTML is empty!");
      return "<html><body><h1>Error: HTML content not loaded</h1></body></html>";
    }
    String html = FPSTR(INDEX_HTML);
    html.replace("%ESP32_IP%", esp32IP);
    Serial.printf("HTML page generated, length: %d bytes\n", html.length());
    return html;
  }

// Motor control functions
void moveForward(int speed) {
  Serial.println("Moving Forward");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void moveBackward(int speed) {
  Serial.println("Moving Backward");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void moveLeft(int speed) {
  Serial.println("Moving Left");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void moveRight(int speed) {
  Serial.println("Moving Right");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void stopMotors() {
  Serial.println("Stopping");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// Helper function to map input angle to 0-180° range based on servo type
float mapServoAngle(int servoNum, float angle) {
  float mappedAngle;
  if (servoNum == 1) { // Base Joint: 0° to 180°
    mappedAngle = map(angle, 0, 180, 0, 180);
  } else if (servoNum == 2) { // Shoulder: -90° to 90°
    mappedAngle = map(angle, -90, 90, 0, 180);
  } else if (servoNum == 3) { // Elbow: -90° to 90° (180° range)
    mappedAngle = map(angle, -90, 90, 0, 180);
  } else if (servoNum == 4) { // Wrist Pitch: -90° to 90°
    mappedAngle = map(angle, -90, 90, 0, 180);
  } else if (servoNum == 5) { // Wrist Roll: -180° to 180°
    mappedAngle = map(angle, -180, 180, 0, 180);
  } else if (servoNum == 6) { // Gripper: 0° to 45°
    mappedAngle = map(angle, 0, 45, 0, 180);
  } else {
    return -1; // Invalid servo number
  }
  return constrain(mappedAngle, 0, 180);
}

// Smooth servo control function with deadband, rate limiting, and easing
void setServoAngle(int servoNum, float angle) {
  if (servoNum < 1 || servoNum > 6) {
    return; // Invalid servo number
  }

  float targetMappedAngle = mapServoAngle(servoNum, angle);
  if (targetMappedAngle < 0) {
    return; // Invalid mapping
  }

  int index = servoNum - 1;
  
  // Check deadband - ignore tiny changes (compare to current target)
  float currentTarget = targetServoAngles[index];
  float change = fabsf(targetMappedAngle - currentTarget);
  
  if (change < SERVO_DEADBAND) {
    return; // Change too small, ignore
  }
  
  // Always update target if deadband passes
  // The smoothing system in updateServoPositions() handles movement rate
  targetServoAngles[index] = targetMappedAngle;
  
  Serial.printf("Servo %d target: %.1f° (change: %.1f°, mapped from %.1f°)\n", 
                servoNum, targetMappedAngle, change, angle);
}

// Smooth easing function that moves servos gradually toward their targets
// This should be called periodically from loop() for smooth continuous motion
void updateServoPositions() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  // Update servos at ~50Hz (every 20ms) for smooth motion
  if ((now - lastUpdate) < 20) {
    return;
  }
  lastUpdate = now;
  
  for (int i = 0; i < 6; i++) {
    float current = currentServoAngles[i];
    float target = targetServoAngles[i];
    float error = target - current;
    
    // If within tolerance, skip
    if (fabsf(error) <= PID_POSITION_TOLERANCE) {
      continue;
    }
    
    // Smooth easing: move a fraction of the way toward target each update
    float step = error * SERVO_EASING_ALPHA;
    
    // Limit maximum step size for safety
    float maxStep = (i == 5) ? 1.5f : 3.0f;  // Gripper moves slower
    if (fabsf(step) > maxStep) {
      step = (step > 0) ? maxStep : -maxStep;
    }
    
    // Update current position
    current += step;
    current = constrain(current, 0.0f, 180.0f);
    currentServoAngles[i] = current;
    
    // Write to servo
    int pulselen = map((int)round(current), 0, 180, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(i, 0, pulselen);
  }
}

// Handle root request
void handleRoot() {
  Serial.println("\n=== Root request received ===");
  Serial.print("Client IP: ");
  Serial.println(server.client().remoteIP());
  digitalWrite(LED_BUILTIN, LOW); // LED on when serving page
  
  String htmlPage = getHtmlPage();
  
  // Check if HTML was generated successfully
  if (htmlPage.length() < 100) {
    Serial.println("ERROR: HTML page too short! Sending error page.");
    String errorPage = "<!DOCTYPE html><html><head><title>Error</title><meta charset='UTF-8'></head><body>";
    errorPage += "<h1>Server Error</h1>";
    errorPage += "<p>HTML content not loaded properly.</p>";
    errorPage += "<p>Server IP: " + WiFi.localIP().toString() + "</p>";
    errorPage += "<p>Please check Serial Monitor for details.</p>";
    errorPage += "</body></html>";
    server.send(500, "text/html; charset=utf-8", errorPage);
  } else {
    // Send response with proper headers
    server.sendHeader("Content-Type", "text/html; charset=utf-8");
    server.sendHeader("Connection", "close");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200, "text/html; charset=utf-8", htmlPage);
    Serial.print("Page sent successfully, length: ");
    Serial.print(htmlPage.length());
    Serial.println(" bytes");
  }
  
  delay(10);
  digitalWrite(LED_BUILTIN, HIGH); // LED off after sending
  Serial.println("Request handled\n");
}

// Handle test/ping endpoint
void handleTest() {
  Serial.println("Test/Ping request received");
  digitalWrite(LED_BUILTIN, LOW);
  String response = "ESP32 Robot Server is running!\n";
  response += "AP SSID: " + String(ap_ssid) + "\n";
  response += "AP IP: " + WiFi.softAPIP().toString() + "\n";
  response += "Connected Stations: " + String(WiFi.softAPgetStationNum()) + "\n";
  response += "Uptime: " + String(millis() / 1000) + " seconds\n";
  server.send(200, "text/plain", response);
  delay(10);
  digitalWrite(LED_BUILTIN, HIGH);
}

// Handle 404 Not Found
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// Handle motor movement
void handleMove() {
  if (server.hasArg("direction")) {
    String direction = server.arg("direction");
    int speed = server.hasArg("speed") ? server.arg("speed").toInt() : 50;
    speed = map(speed, 0, 100, 0, 255);
    if (direction == "forward") {
      moveForward(speed);
    } else if (direction == "backward") {
      moveBackward(speed);
    } else if (direction == "left") {
      moveLeft(speed);
    } else if (direction == "right") {
      moveRight(speed);
    } else if (direction == "stop") {
      stopMotors();
    }
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// Handle light control
void handleLight() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "on") {
      digitalWrite(LIGHT_PIN, HIGH);
      Serial.println("Light turned ON");
      server.send(200, "text/plain", "OK");
    } else if (state == "off") {
      digitalWrite(LIGHT_PIN, LOW);
      Serial.println("Light turned OFF");
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Bad Request: state must be 'on' or 'off'");
    }
  } else {
    server.send(400, "text/plain", "Bad Request: missing 'state' parameter");
  }
}

// Handle servo and other commands
void handleCommand() {
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    if (cmd.startsWith("SERVO")) {
      int firstColon = cmd.indexOf(':');
      int secondColon = cmd.indexOf(':', firstColon + 1);
      if (firstColon != -1 && secondColon != -1) {
        int servoNum = cmd.substring(firstColon + 1, secondColon).toInt();
        float angle = cmd.substring(secondColon + 1).toFloat();
        setServoAngle(servoNum, angle);
        server.send(200, "text/plain", "OK");
      } else {
        server.send(400, "text/plain", "Invalid SERVO command format");
      }
    } else if (cmd.startsWith("SPEED")) {
      int colonIndex = cmd.indexOf(':');
      if (colonIndex != -1) {
        int speed = cmd.substring(colonIndex + 1).toInt();
        speed = map(speed, 0, 100, 0, 255);
        // Apply to PWM channels
        ledcWrite(PWM_CHANNEL_A, speed);
        ledcWrite(PWM_CHANNEL_B, speed);
        server.send(200, "text/plain", "OK");
      } else {
        server.send(400, "text/plain", "Invalid SPEED command format");
      }
    } else if (cmd == "RESET") {
      // Move servos to zero position sequentially to reduce power spikes
      for (int i = 1; i <= 6; i++) {
        setServoAngle(i, 0);
        delay(150); // Delay between servos for stability
      }
      stopMotors();
      server.send(200, "text/plain", "OK");
    } else if (cmd == "HOME") {
      float homePositions[] = {0, 0, 0, 0, 0, 0};
      // Move servos to home position sequentially to reduce power spikes
      for (int i = 0; i < 6; i++) {
        setServoAngle(i + 1, homePositions[i]);
        delay(150); // Delay between servos for stability
      }
      server.send(200, "text/plain", "OK");
    } else if (cmd == "DEMO") {
      float sequences[][3] = {
        {0, 90, 1000}, {1, -30, 1000}, {2, 45, 1000}, {3, -20, 1000}, {4, 45, 1000},
        {5, 45, 500}, {5, 22, 500}, {5, 45, 500}, {5, 22, 500},
        {0, 0, 1000}, {1, 0, 1000}, {2, 0, 1000}, {3, 0, 1000}, {4, 0, 1000}, {5, 0, 500}
      };
      for (int i = 0; i < 15; i++) {
        setServoAngle(sequences[i][0] + 1, sequences[i][1]);
        delay(sequences[i][2]);
      }
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Unknown command");
    }
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500); // Wait for serial to initialize
  
  // Initialize built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn on LED to show ESP32 is powered
  
  // Blink LED 3 times during startup
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }
  
  Serial.println("\n\n========================================");
  Serial.println("ESP32 Robot Control System Starting...");
  Serial.println("========================================\n");
  
  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  // analogWrite is available on many ESP32 board definitions via Arduino core's compatibility layer
  stopMotors();
  
  // Initialize light relay pin
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW); // Start with light OFF
  Serial.println("Light relay initialized on pin 4 (OFF)");
  initializePidControllers();
  // Initialize I2C and PWM driver
  Serial.println("Initializing I2C and PWM driver...");
  Wire.begin(21, 20); // SDA=21, SCL=20
  delay(100);
  
  if (pwm.begin()) {
    pwm.setPWMFreq(50);
    Serial.println("PWM driver initialized successfully");
    
    // Initialize all servos to home position (0°) smoothly
    Serial.println("Initializing servos to home position...");
    // Initialize target angles to match current (home position)
    for (int i = 0; i < 6; i++) {
      currentServoAngles[i] = 0.0f;
      targetServoAngles[i] = 0.0f;
      int pulselen = map(0, 0, 180, SERVO_MIN, SERVO_MAX);
      pwm.setPWM(i, 0, pulselen);
      delay(100); // Small delay between servos to reduce power spikes
    }
    Serial.println("Servos initialized to home position");
  } else {
    Serial.println("WARNING: PWM driver initialization failed! Servo control may not work.");
    Serial.println("Continuing without PWM driver...");
  }
  // Start WiFi Access Point
  Serial.println("\nConfiguring ESP32 as a dedicated access point...");
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP);
  delay(100);

  if (!WiFi.softAPConfig(ap_ip, ap_gateway, ap_subnet)) {
    Serial.println("Failed to configure static AP IP settings, using defaults.");
  }

  bool apStarted = (ap_password[0] == '\0') ? WiFi.softAP(ap_ssid) : WiFi.softAP(ap_ssid, ap_password);
  if (!apStarted) {
    Serial.println("Failed to start SoftAP! Rebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  IPAddress actualIP = WiFi.softAPIP();

  Serial.println("\n=== Access Point ready! ===");
  Serial.print("Network Name (SSID): ");
  Serial.println(ap_ssid);
  Serial.println("Security: OPEN (no password)");
  Serial.print("AP IP address: ");
  Serial.println(actualIP);
  Serial.print("Connected Stations (current): ");
  Serial.println(WiFi.softAPgetStationNum());
  Serial.println("=========================================\n");
  
  // LED fast blink to indicate AP is ready
  for(int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
  
  // Set up web server routes BEFORE starting server
  server.on("/", handleRoot);
  server.on("/test", handleTest);
  server.on("/ping", handleTest);
  server.on("/move", handleMove);
  server.on("/command", handleCommand);
  server.on("/light", handleLight);
  server.on("/assets/three.min.js", handleThreeJS);
  server.on("/assets/OrbitControls.js", handleOrbitControlsJS);
  server.onNotFound(handleNotFound);
  
  // Start server on port 80
  server.begin();
  
  delay(500); // Give server time to fully start
  
  Serial.println("Server routes configured:");
  Serial.println("  GET  /");
  Serial.println("  GET  /test");
  Serial.println("  GET  /ping");
  Serial.println("  GET  /move");
  Serial.println("  GET  /command");
  Serial.println("  GET  /light");
  
  Serial.println("=========================================");
  Serial.println("Web server started on port 80");
  Serial.println("=========================================");
  Serial.println("Access the web interface at:");
  Serial.print("  http://");
  Serial.print(actualIP);
  Serial.println("/");
  Serial.println("\nTest endpoints:");
  Serial.print("  http://");
  Serial.print(actualIP);
  Serial.println("/test");
  Serial.print("  http://");
  Serial.print(actualIP);
  Serial.println("/ping");
  Serial.println("=========================================");
  Serial.println("Ready to accept connections!");
  Serial.println("LED will blink on each request.");
  Serial.println("=========================================\n");
  
  // Fast blink 3 times to indicate ready
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
  
  // Keep LED on to show system is ready and connected
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  server.handleClient();
  
  // Update servo positions smoothly (non-blocking easing)
  updateServoPositions();
  
  // Slow blink LED every 3 seconds to show system is alive
  static unsigned long lastBlink = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastBlink > 3000) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    lastBlink = currentTime;
  }
}
