# HomePod S3 – Smart Desk Companion
![IMG_8774](https://github.com/user-attachments/assets/a5b2c33d-e3e2-4916-8235-98ffd8162230)


Hi, I’m Rohan, and this is our hackathon project **HomePod S3 — Smart Desk Companion**.

The motivation behind this device was simple yet powerful:

> “If a small smart device sits permanently on our desk, what meaningful things can it actually help us with every single day?”

Instead of building a trendy IoT gadget, we focused on something that genuinely improves:
- ✔ Studying  
- ✔ Work routines  
- ✔ Comfort  
- ✔ Medication habits  
- ✔ Daily productivity  

To make the device more appealing and desk-friendly, we also designed and 3D-printed a custom outer enclosure with a **gear-shaped pen holder**.  
This not only improves the overall aesthetics but also adds practical utility as a small desk organizer.

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
![WhatsApp Image 2026-02-07 at 1 08 09 PM (1)](https://github.com/user-attachments/assets/5c6a8272-8d76-43a5-9e8a-eeeb9621f409)

📌 **Benefit:**  
Tasks stay visible on the desk all day, unlike phone apps that cause distractions.

### 2️⃣ Study Timer (Pomodoro-style)  
![WhatsApp Image 2026-02-07 at 1 08 11 PM (2)](https://github.com/user-attachments/assets/8d1481b7-441d-4030-9894-1a310ad711a0)

📌 **Benefit:**  
Encourages focused study sessions without using a smartphone.

### 3️⃣ Medicine Reminder  
![WhatsApp Image 2026-02-07 at 1 27 31 PM](https://github.com/user-attachments/assets/05908f56-e3d8-4f49-b93a-9030fdcdfbbf)

📌 **Benefit:**  
Audio + visual alerts help avoid missed doses.

### 4️⃣ Touch-based Light & Fan Controls  
![WhatsApp Image 2026-02-07 at 1 19 41 PM](https://github.com/user-attachments/assets/72f36066-2b5d-4af4-8093-f081c128740e)

📌 **Benefit:**  
No app, no cloud, no login.

![WhatsApp Image 2026-02-07 at 1 32 31 PM](https://github.com/user-attachments/assets/935024eb-3967-4cb0-8be0-66f6676d7a86)


### 5️⃣ Temperature Display  
📌 **Benefit:**  
Shows room temperature during long study/work sessions.

### 6️⃣ Lock Screen with Slide-to-Unlock  
📌 **Benefit:**  
Prevents accidental touches and feels like a real product.

### 7️⃣ Web-Based Remote UI  
![WhatsApp Image 2026-02-07 at 1 08 10 PM](https://github.com/user-attachments/assets/57b1cb1d-1a95-4830-a901-4ec21edb5cb1)

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
- Web server + UI synchronization  
- Sensors + timers + scheduling  
- Remote debugging & collaboration  
- First-time use of ESP32-S3 Box3  

By teamwork and persistence, we successfully built a functional and enjoyable product.

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
- PWM-based fan speed control  

### ➤ Relay Module  
- Light 1, Light 2, Light 3 control  

### ➤ Buzzer  
- Timer completion + medicine alerts  

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
8. Ensure all components share common GND  
9. Power everything using USB-C  
10. Double-check wiring before powering on  

---

## 📸 Operation & Usage (With Detailed UI Explanation)

### 🔹 Navigation Between Screens  
<img width="1280" height="720" alt="Subheading" src="https://github.com/user-attachments/assets/80576152-2b2c-401f-8fcf-08f34035a28d" />

`<` and `>` buttons on the screen are used to navigate between:
- Controller Screen  
- Timer Screen  
- To-Do List Screen  
- Medicine List Screen  

---

### 🔹 Controller Screen (Lights & Fan)

- Control Light 1, Light 2, Light 3 ON/OFF  
- Control Fan ON/OOFF  

📌 **Fan Speed Popup:**  
![WhatsApp Image 2026-02-07 at 1 34 28 PM](https://github.com/user-attachments/assets/0ccb9d62-e085-4ce9-bff7-321a44a5d0ca)

Double-tap the Fan icon → a popup appears  
Use `+` and `-` to increase or decrease fan speed  


---

### 🔹 Timer Screen  
![WhatsApp Image 2026-02-07 at 1 08 11 PM (2)](https://github.com/user-attachments/assets/af1c334d-1d43-40f7-8a51-45304a75bbef)

- Set time in **HH:MM:SS** format  
- `+` / `-` adjusts hours, minutes, seconds  
- ▶️ Play → Start timer  
- ⏸️ Pause → Stop timer  

When timer ends:
![WhatsApp Image 2026-02-07 at 1 08 10 PM (2)](https://github.com/user-attachments/assets/8f67c735-a5f3-4be2-81c6-13199cb580a9)

- Buzzer sounds  
- Big **TIMER DONE** screen appears


---

### 🔹 To-Do List Screen  
![WhatsApp Image 2026-02-07 at 1 08 09 PM (1)](https://github.com/user-attachments/assets/490b599a-45b7-4e4a-9fc9-caa77e96d097)

- Tasks added from Web UI appear here  
- Tap any task to view full text  
- Checkbox on the side deletes the task (task completed)  

![WhatsApp Image 2026-02-07 at 1 08 09 PM](https://github.com/user-attachments/assets/78b80de6-bce7-4dbc-abc5-eb3d33644658)


---

### 🔹 Medicine List Screen  
- Medicines added from Web UI appear here  
- Ticking the checkbox stops further reminders for that dose  


---![WhatsApp Image 2026-02-07 at 1 27 31 PM](https://github.com/user-attachments/assets/e9783bf1-6ea8-4b98-bf0b-498d09eee8d9)


### 🔹 Lock Screen  
- Auto-lock after idle  
- Shows time, temperature, slide-to-unlock  
- Wake via touch or voice  


![WhatsApp Image 2026-02-07 at 1 32 31 PM](https://github.com/user-attachments/assets/c0e8fc77-67e8-451d-b779-5ce31059c8f0)

---

## 🌐 Web UI (Local Web Dashboard)
![WhatsApp Image 2026-02-07 at 1 08 10 PM](https://github.com/user-attachments/assets/7398ab65-d862-4c80-b668-e065c79d97c0)

### 🔹 Light & Fan Controls  
<img width="1280" height="720" alt="Subheading (2)" src="https://github.com/user-attachments/assets/1735c22c-6d6c-4723-ac47-64d385cc1e30" />

- Light 1, Light 2, Light 3 ON/OFF  
- Fan ON/OFF  
- Fan speed set via **dropdown menu**

---

### 🔹 Add Task Section  
<img width="1280" height="720" alt="Subheading (1)" src="https://github.com/user-attachments/assets/520adbab-5b81-4468-8cfb-7c64426ba30f" />

- Add tasks (max 5 tasks)  
- **D button** deletes a task  
- Changes sync instantly to device  

---

### 🔹 Medicine List Section  
<img width="1280" height="720" alt="Subheading (3)" src="https://github.com/user-attachments/assets/966be32a-4ba7-468f-898c-fa0bc57da112" />

- Add medicine name + time + AM/PM  
- **Add** button to save  
- **D button** deletes medicine  
- Real-time sync with device  

---

## 🗣️ Voice Commands (Offline)

- “Hello HomePod”  
- “Light 1 ON / OFF”  
- “Light 2 ON / OFF”  
- “Light 3 ON / OFF”  
- “Fan ON / OFF”  
- “Fan speed to 3”  
- “Open Timer”  
- “Open List”  
- “Open Medicine List”  

---

## 🧾 Conclusion

HomePod S3 evolved from a simple desk controller into a local, offline-first smart assistant for productivity, comfort, and daily routine management.  
It supports **touch, voice, and web-based interaction** without relying on cloud services or mobile apps.
