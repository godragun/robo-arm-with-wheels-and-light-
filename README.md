# ESP32 Robotic Arm Car with Camera and Login

<p align="center">
  <img src="https://raw.githubusercontent.com/godragun/robo-arm-with-wheels-and-light-/main/roboarm.jpeg" alt="Robo Arm" width="420" />
  &nbsp;&nbsp;
  <img src="https://raw.githubusercontent.com/godragun/robo-arm-with-wheels-and-light-/main/roboarmcirciuit.jpeg" alt="Robo Arm Circuit" width="420" />
</p>

A comprehensive ESP32-based robotic arm car control system with web interface, featuring 3D visualization, motor control, servo manipulation, and camera integration.

## Features

- 🤖 **6-DOF Robotic Arm Control** - Precise servo control with smooth motion and PID control
- 🚗 **Car Movement** - Forward, backward, left, and right movement with speed control
- 💡 **Light/Relay Control** - Remote control of onboard lighting system
- 📱 **Web Interface** - Modern, responsive web UI with 3D visualization
- 📷 **Camera Integration** - Camera support for remote monitoring
- 🔐 **Login System** - Secure access control
- 📶 **WiFi Access Point** - Creates its own WiFi network for direct connection
- 🎨 **3D Visualization** - Real-time 3D model of the robotic arm using Three.js

## Hardware Requirements

### Main Components
- **ESP32 Development Board** (any variant)
- **Adafruit PCA9685 PWM Servo Driver** (I2C address 0x40)
- **6x Servo Motors** (for robotic arm joints)
- **4x DC Motors** with motor driver (L298N or similar)
- **Camera Module** (compatible with ESP32)
- **Relay Module** (for light control)
- **Power Supply** (adequate for all components)

### Pin Connections

#### Motor Control Pins
- `IN1` → GPIO 14
- `IN2` → GPIO 17
- `IN3` → GPIO 18
- `IN4` → GPIO 19
- `ENA` → GPIO 13 (PWM)
- `ENB` → GPIO 12 (PWM)

#### Other Pins
- `LED_BUILTIN` → GPIO 2 (Built-in LED)
- `LIGHT_PIN` → GPIO 4 (Relay/Light control)
- `SDA` → GPIO 21 (I2C for PWM driver)
- `SCL` → GPIO 20 (I2C for PWM driver)

### Servo Configuration
- **Servo 1 (Base Joint)**: 0° to 180°
- **Servo 2 (Shoulder)**: -90° to 90°
- **Servo 3 (Elbow)**: -90° to 90°
- **Servo 4 (Wrist Pitch)**: -90° to 90°
- **Servo 5 (Wrist Roll)**: -180° to 180°
- **Servo 6 (Gripper)**: 0° to 45°

## Software Requirements

- **Arduino IDE** (1.8.x or 2.x) or **PlatformIO**
- **ESP32 Board Support** (via Arduino Board Manager)
- **Required Libraries**:
  - `WiFi` (included with ESP32)
  - `WebServer` (included with ESP32)
  - `Wire` (included with ESP32)
  - `Adafruit_PWMServoDriver` (install via Library Manager)

## Installation

1. **Clone or download this repository**

2. **Install Arduino IDE and ESP32 Board Support**
   - Open Arduino IDE
   - Go to `File` → `Preferences`
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools` → `Board` → `Boards Manager`
   - Search for "ESP32" and install "esp32 by Espressif Systems"

3. **Install Required Libraries**
   - Go to `Sketch` → `Include Library` → `Manage Libraries`
   - Search and install:
     - `Adafruit PWM Servo Driver Library`

4. **Select Board and Port**
   - Go to `Tools` → `Board` → Select your ESP32 board variant
   - Go to `Tools` → `Port` → Select the COM port where ESP32 is connected

5. **Upload the Code**
   - Click the Upload button (→) in Arduino IDE
   - Wait for compilation and upload to complete

## Configuration

### WiFi Access Point Settings

The ESP32 creates its own WiFi network. Default settings:

- **SSID**: `robo arm`
- **Password**: None (open network)
- **IP Address**: `192.168.4.1`

To change these settings, modify the following in `roboarmcarwithcamandlogin.ino`:

```cpp
const char* ap_ssid = "robo arm";   // Change network name
const char* ap_password = "";        // Add password if needed
IPAddress ap_ip(192, 168, 4, 1);     // Change IP if needed
```

### Servo Tuning

The system includes PID control and smoothing for smooth servo motion. You can adjust these parameters:

```cpp
const float PID_KP = 0.5f;              // Proportional gain
const float PID_KI = 0.01f;             // Integral gain
const float PID_KD = 0.12f;             // Derivative gain
const float SERVO_EASING_ALPHA = 0.15f; // Motion smoothing
```

## Usage

1. **Power On the Robot**
   - Connect power supply to ESP32
   - Wait for LED to blink 3 times (startup sequence)
   - LED will blink 5 times when WiFi AP is ready

2. **Connect to WiFi**
   - On your phone/computer, search for WiFi network named "robo arm"
   - Connect (no password required)
   - Wait for connection confirmation

3. **Access Web Interface**
   - Open a web browser
   - Navigate to `http://192.168.4.1`
   - The control interface should load

4. **Control the Robot**
   - Use the web interface to:
     - Control car movement (forward, backward, left, right)
     - Adjust servo positions using sliders or 3D visualization
     - Control the gripper
     - Turn light on/off
     - Run demo sequences

## API Endpoints

The web server provides the following REST API endpoints:

- `GET /` - Main web interface
- `GET /test` or `/ping` - Server status check
- `GET /move?direction={forward|backward|left|right|stop}&speed={0-100}` - Control car movement
- `GET /command?cmd=SERVO:{servo_num}:{angle}` - Set servo angle
- `GET /command?cmd=RESET` - Reset all servos to zero position
- `GET /command?cmd=HOME` - Move all servos to home position
- `GET /command?cmd=DEMO` - Run demo sequence
- `GET /light?state={on|off}` - Control light/relay

### Example API Calls

```bash
# Move forward at 50% speed
http://192.168.4.1/move?direction=forward&speed=50

# Set servo 1 to 90 degrees
http://192.168.4.1/command?cmd=SERVO:1:90

# Turn light on
http://192.168.4.1/light?state=on
```

## Project Structure

```
roboarmcarwithcamandlogin/
├── roboarmcarwithcamandlogin.ino  # Main Arduino sketch
├── index_html.h                   # Web interface HTML (PROGMEM)
├── three_js_asset.h               # Three.js library (PROGMEM)
├── orbitcontrols_js_asset.h       # OrbitControls library (PROGMEM)
└── README.md                      # This file
```

## Features in Detail

### Smooth Servo Control
- PID-based position control for precise movement
- Easing algorithm for smooth motion transitions
- Deadband filtering to reduce jitter
- Rate limiting to prevent sudden movements

### Motor Control
- PWM speed control (0-100%)
- Bidirectional movement (forward, backward, left, right)
- Emergency stop functionality

### Web Interface
- Modern, responsive design
- Dark/light mode support
- Real-time 3D visualization of robotic arm
- Touch-friendly controls for mobile devices
- Live status indicators

## Troubleshooting

### ESP32 Not Connecting
- Check USB cable (data-capable cable required)
- Install correct USB drivers (CP2102 or CH340)
- Select correct COM port in Arduino IDE

### WiFi Not Appearing
- Check Serial Monitor (115200 baud) for error messages
- Verify ESP32 has sufficient power
- Try resetting the ESP32

### Servos Not Moving
- Verify I2C connection (SDA/SCL pins)
- Check PWM driver power supply
- Verify servo connections to PWM driver
- Check Serial Monitor for initialization errors

### Web Interface Not Loading
- Ensure you're connected to "robo arm" WiFi network
- Try accessing `http://192.168.4.1/test` first
- Clear browser cache
- Check Serial Monitor for server errors

## Safety Notes

⚠️ **Important Safety Considerations:**

- Ensure adequate power supply for all motors and servos
- Be careful with servo limits to avoid mechanical damage
- Keep fingers away from moving parts during operation
- Use appropriate motor drivers rated for your motors
- The system creates an open WiFi network by default - consider adding password protection for security

## License

This project is provided as-is for educational and personal use.

## Contributing

Contributions, issues, and feature requests are welcome!

## Acknowledgments

- Adafruit for the PWM Servo Driver library
- Three.js for 3D visualization capabilities
- ESP32 community for excellent documentation

## Version History

- **v1.0** - Initial release with basic functionality
  - 6-DOF robotic arm control
  - Car movement control
  - Web interface with 3D visualization
  - Light/relay control
  - WiFi Access Point mode

---

**Note**: This project requires basic knowledge of Arduino programming and electronics. Make sure to double-check all connections before powering on the system.
