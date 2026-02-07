# HomePod S3 – Smart Desk Companion

Hi, I’m Rohan, and this is our hackathon project **HomePod S3 — Smart Desk Companion**.

The motivation behind this device was simple yet powerful:

> “If a small smart device sits permanently on our desk, what meaningful things can it actually help us with every single day?”

Instead of building a trendy IoT gadget, we focused on something that genuinely improves:
- ✔ Studying  
- ✔ Work routines  
- ✔ Comfort  
- ✔ Medication habits  
- ✔ Daily productivity  

During development, our team wasn’t physically together. I worked on the hardware, while Ashwin and Aayush joined remotely over Google Meet for debugging, UI decisions, feature validation, and brainstorming.  
Despite being remote, the collaboration was smooth and enjoyable.

---

## 🚀 Feature Evolution & Practical Benefits

Initially, the idea was limited to:
- Touch-based Light Controls  
- Fan Speed Control  
- Offline Voice Commands  

As we focused on real-world usability, the scope naturally expanded:

### 1️⃣ To-Do List / Task Manager  
📌 **Benefit:**  
Tasks stay visible on the desk all day, unlike phone apps that cause distractions.

### 2️⃣ Study Timer (Pomodoro-style)  
📌 **Benefit:**  
Encourages focused study sessions without using a smartphone.

### 3️⃣ Medicine Reminder  
📌 **Benefit:**  
Audio + visual alerts help avoid missed doses.

### 4️⃣ Touch-based Light & Fan Controls  
📌 **Benefit:**  
No app, no cloud, no login.

### 5️⃣ Temperature Display  
📌 **Benefit:**  
Shows room temperature during long study/work sessions.

### 6️⃣ Lock Screen with Slide-to-Unlock  
📌 **Benefit:**  
Prevents accidental touches and feels like a real product.

### 7️⃣ Web-Based Remote UI  
Works on phone, desktop, and tablet over local Wi-Fi.

📌 **Benefits:**  
- Control lights & fan remotely  
- Adjust fan speed  
- Add medicines and to-dos  
- Delete items in real time  
- Works without cloud or internet  
- Minimal distractions  

---

## 👥 Who Can Use It?

- ✔ Students — study timer, tasks, comfort  
- ✔ Working professionals — reminders, remote control  
- ✔ Elderly — medicine reminders + voice control  
- ✔ Anyone avoiding distractions — no phone app required  

---

## ⚙️ Challenges Faced & Final Words

This project involved integrating multiple subsystems:
- Voice module integration  
- Display + touch UI  
- Web server + UI sync  
- Sensors + timers  
- Remote collaboration  
- First-time use of ESP32-S3 Box3  

By teamwork and persistence, we successfully built a working and enjoyable product.

Special thanks to **CircuitDigest & Digi-Key** for organizing the hackathon and providing hardware and documentation support.

---

## 🧩 Components Used & Why

### ➤ ESP32-S3 Box3 + Dock  
Chosen because it provides:
- Capacitive touch display  
- Wi-Fi  
- Dual-core MCU  
- Compact desk-friendly enclosure  
- USB-C power & programming  

Dock Board benefits:
- Exposes GPIO pins  
- UART, PWM & GPIO access  
- Clean wiring & easy flashing  

### ➤ DF2301Q Offline Voice Module  
- Offline  
- Low latency  
- UART interface  
- Privacy-friendly  

### ➤ DHT11 Temperature Sensor  
- Simple digital interface  
- Easy ESP32 integration  

### ➤ DC Motor (Fan Simulation)  
- Used to simulate fan speed via PWM  

### ➤ Relay Module  
- Light 1, Light 2, Light 3 control  

### ➤ Buzzer  
- Timer & medicine alerts  

---

## 🔌 Circuit Connections

### DF2301Q → ESP32-S3 Box3  
- RX → GPIO 44  
- TX → GPIO 43  
- GND → GND  
- VCC → 3V  

### DHT11 → ESP32-S3 Box3  
- GND → GND  
- VCC → VCC  
- DATA → GPIO 42  

### Relay Module → ESP32-S3 Box3  
- GND → GND  
- VCC → 3V  
- IN1 → GPIO 10  
- IN2 → GPIO 14  
- IN3 → GPIO 13  

### Buzzer → ESP32-S3 Box3  
- GND → GND  
- VCC → GPIO 41  

---

## 🛠️ Hardware Assembly (Basic Steps)

1. Mount ESP32-S3 Box3 into Dock Board  
2. Connect relays to GPIO 10, 14, 13  
3. Connect DHT11 to GPIO 42  
4. Connect motor to GPIO 21 (PWM)  
5. Connect buzzer to GPIO 41  
6. Connect DF2301Q via UART  
7. Switch DF2301Q from I2C → UART mode  
8. Ensure common GND  
9. Power via USB-C  
10. Double-check wiring  
