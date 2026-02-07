// ===================== LIBRARIES =====================
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include <DHT.h>

#define LGFX_ESP32_S3_BOX_V3
#include <LGFX_AUTODETECT.hpp>
#include <LovyanGFX.hpp>

#include "DFRobot_DF2301Q.h"

// ===================== DF2301Q UART =====================
#define ASR_RX 44   // DF2301Q TX
#define ASR_TX 43   // DF2301Q RX
DFRobot_DF2301Q_UART asr(&Serial1, ASR_RX, ASR_TX);
// ===== FAN PWM =====
#define FAN_PWM_PIN 21
#define FAN_PWM_FREQ 25000   // fan ke liye best
#define FAN_PWM_RES  8       // 0–255

// ===================== DISPLAY =====================
static LGFX lcd;
WebServer server(80);
// ===================== DHT SENSOR =====================
#define DHTPIN 42        // 🔥 GPIO 42
#define DHTTYPE DHT11    // agar DHT22 hai toh DHT22

DHT dht(DHTPIN, DHTTYPE);

float roomTemp = NAN;
float roomHum  = NAN;
uint32_t lastDHTRead = 0;


// ===================== WIFI =====================
#define WIFI_SSID "SHRRADHA"
#define WIFI_PASS "Rahul@2001"

// ===================== DISPLAY =====================
#define W 320
#define H 240
#define HEADER_H 40
// ===================== BUZZER =====================
#define BUZZER_PIN 41
bool timerBeepPlayed = false;
// ===== GPIO OUTPUTS =====
#define GPIO_LIGHT1 10
#define GPIO_LIGHT2 14
#define GPIO_LIGHT3 13





// ===== SLIDER STATE =====
int sliderX = 45;          // knob current X
const int sliderStartX = 45;
const int sliderEndX   = 235;  // barX + barW - knobWidth


// ===================== STATES =====================
bool light1=false, light2=false, light3=false, fanOn=false;
int fanSpeed=3;
bool medBeepPlayed = false;
bool touchLock=false;
bool uiDirty=true;

// ===================== SCREENS =====================
enum Screen { SCREEN_MAIN, SCREEN_TIMER, SCREEN_TASKS, SCREEN_TASK_DETAIL, SCREEN_MEDICINES };
Screen currentScreen = SCREEN_MAIN;
// ===================== LOCK SCREEN =====================
bool isLocked = false;
uint32_t lastUserInteraction = 0;

// swipe detection
int swipeStartX = -1;
uint32_t swipeStartTime = 0;


// ===================== TIMER =====================
int t_h=0,t_m=0,t_s=0;
int selectedField=0;
bool timerRunning=false;
bool timerDone=false;
uint32_t lastTick=0;
uint32_t timerDoneMillis=0;

// ===================== TASKS =====================
#define MAX_TASKS 5
String tasks[MAX_TASKS];
int taskCount=0;
int selectedTask=-1;
uint32_t taskDetailMillis=0;

// ===================== MEDICINES =====================
#define MAX_MED 5
struct Medicine{
  String name;
  int hour;
  int minute;
  bool pm;
  bool taken;
};
Medicine meds[MAX_MED];
int medCount=0;

bool medPopup=false;
String popupMedName="";
uint32_t popupMillis=0;
uint32_t lastMedTrigger=0;

// ===================== FAN =====================
bool fanPopup=false;
uint32_t lastInteraction=0;
uint32_t lastTapTime=0;

// ===================== TIME =====================
struct tm timeinfo;
bool timeValid=false;
uint32_t lastTimeUpdate=0;

// ===================== VOICE =====================
void handleVoiceCommand(uint8_t id){
  Serial.print("[VOICE] CMDID = ");
  Serial.println(id);

  // 🔥 VOICE COUNTS AS USER ACTIVITY
  lastUserInteraction = millis();

  // ===== VOICE UNLOCK =====
  if(isLocked && (id == 1 || id == 2)){
    isLocked = false;
    uiDirty = true;
    return;
  }


  switch(id){

    // -------- LIGHTS ON --------
    case 5:   // Light 1 ON
      light1 = true;
      break;

    case 7:   // Light 2 ON
      light2 = true;
      break;

    case 9:   // Light 3 ON
      light3 = true;
      break;

    // -------- LIGHTS OFF --------
    case 6:   // Light 1 OFF
      light1 = false;
      break;

    case 8:   // Light 2 OFF
      light2 = false;
      break;

    case 10:  // Light 3 OFF
      light3 = false;
      break;

    // -------- FAN ON / OFF --------
    case 11:  // Fan ON
      fanOn = true;
      break;

    case 12:  // Fan OFF
      fanOn = false;
      break;

    // -------- FAN SPEED --------
    case 17:  // Speed 1
      fanSpeed = 1;
      fanOn = true;
      break;

    case 18:  // Speed 2
      fanSpeed = 2;
      fanOn = true;
      break;

    case 19:  // Speed 3
      fanSpeed = 3;
      fanOn = true;
      break;

    case 20:  // Speed 4
      fanSpeed = 4;
      fanOn = true;
      break;

    case 21:  // Speed 5
      fanSpeed = 5;
      fanOn = true;
      break;

    // -------- SCREEN NAVIGATION --------
    case 13:  // Open Timer
      currentScreen = SCREEN_TIMER;
      break;

    case 14:  // Open Controls (Home)
      currentScreen = SCREEN_MAIN;
      break;

    case 15:  // Open Task List
      currentScreen = SCREEN_TASKS;
      break;

    case 16:  // Open Medicine List
      currentScreen = SCREEN_MEDICINES;
      break;

    default:
      Serial.println("[VOICE] Unknown CMDID");
      return;
  }

  uiDirty = true;   // 🔥 refresh UI
  syncGPIO();
  updateFanPWM();
}


// ===================== BOOT =====================
void bootAnimation(){
  lcd.fillScreen(TFT_BLACK);
  lcd.setFont(&fonts::FreeSansBold24pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(middle_center);
  String s="HomePod",b="";
  for(char c:s){
    b+=c;
    lcd.fillScreen(TFT_BLACK);
    lcd.drawString(b,W/2,H/2);
    delay(120);
  }
}

// ===================== TIME INIT =====================
void initTime(){
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  uint32_t t=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t<8000) delay(200);
  if(WiFi.status()!=WL_CONNECTED) return;
  configTime(5*3600+1800,0,"pool.ntp.org");
  if(getLocalTime(&timeinfo)) timeValid=true;
}

// ===================== HEADER =====================
void drawHeader(const char* title){
  lcd.fillRect(0,0,W,HEADER_H,TFT_WHITE);
  lcd.setFont(&fonts::FreeSansBold9pt7b);
  lcd.setTextColor(TFT_BLACK);

  lcd.setTextDatum(middle_left);
  lcd.drawString("<",5,HEADER_H/2);
  lcd.drawString(title,30,HEADER_H/2);

  if(timeValid){
    char buf[12];
    strftime(buf,sizeof(buf),"%I:%M %p",&timeinfo);
    lcd.setTextDatum(middle_right);
    lcd.drawString(buf,W-25,HEADER_H/2);
  }

  lcd.setTextDatum(middle_right);
  lcd.drawString(">",W-5,HEADER_H/2);
}

// ===================== BOX =====================
void drawBox(int x,int y,const char* label,bool state){
  lcd.fillRoundRect(x,y,130,55,10,state?TFT_BLUE:TFT_BLACK);
  lcd.drawRoundRect(x,y,130,55,10,TFT_WHITE);
  lcd.setFont(&fonts::FreeSansBold9pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(middle_center);
  lcd.drawString(label,x+65,y+28);
}

// ===================== MAIN =====================
void drawMain(){
  lcd.fillScreen(TFT_BLACK);
  drawHeader("Controls");

  drawBox(20,55,"Light 1",light1);
  drawBox(170,55,"Light 2",light2);
  drawBox(20,135,"Light 3",light3);
  drawBox(170,135,"Fan",fanOn);

  if(fanPopup){
    lcd.fillRoundRect(60,70,200,100,20,TFT_BLACK);
    lcd.drawRoundRect(60,70,200,100,20,TFT_BLUE);
    lcd.setFont(&fonts::FreeSansBold24pt7b);
    lcd.setTextColor(TFT_BLUE);
    lcd.setTextDatum(middle_center);
    lcd.drawString(String(fanSpeed),W/2,110);
    lcd.setFont(&fonts::FreeSansBold18pt7b);
    lcd.drawString("-",90,150);
    lcd.drawString("+",230,150);
  }
}
void syncGPIO(){
  digitalWrite(GPIO_LIGHT1, light1 ? LOW : HIGH);
  digitalWrite(GPIO_LIGHT2, light2 ? LOW : HIGH);
  digitalWrite(GPIO_LIGHT3, light3 ? LOW : HIGH);
  updateFanPWM();
}


// ===================== TIMER =====================
void drawTimer(){
  lcd.fillScreen(TFT_BLACK);
  drawHeader("Timer");

  lcd.setFont(&fonts::FreeSansBold18pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(middle_center);

  char buf[12];
  sprintf(buf,"%02d:%02d:%02d",t_h,t_m,t_s);
  lcd.drawString(buf,W/2,95);

  int bx[3]={118,160,202};
  lcd.drawLine(bx[selectedField]-12,118,bx[selectedField]+12,118,TFT_WHITE);

  lcd.drawRoundRect(40,160,50,40,8,TFT_RED);
  lcd.drawRoundRect(230,160,50,40,8,TFT_GREEN);
  lcd.drawRoundRect(135,160,50,40,8,TFT_WHITE);

  lcd.drawString("-",65,180);
  lcd.drawString("+",255,180);

  if(timerRunning) lcd.drawString("||",160,180);
  else lcd.fillTriangle(145,170,145,190,175,180,TFT_WHITE);
}

// ===================== TIMER DONE =====================
void drawTimerDone(){
  lcd.fillScreen(TFT_GREEN);
  lcd.setFont(&fonts::FreeSansBold24pt7b);
  lcd.setTextColor(TFT_BLACK);
  lcd.setTextDatum(middle_center);
  lcd.drawString("TIMER DONE",W/2,H/2);
}

// ===================== TASKS =====================
void drawTasks(){
  lcd.fillScreen(TFT_BLACK);
  drawHeader("To Do");

  lcd.setFont(&fonts::FreeSansBold12pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(top_left);

  if(taskCount==0){
    lcd.drawString("No task yet",90,120);
    return;
  }

  for(int i=0;i<taskCount;i++){
    int y=HEADER_H+15+i*35;
    lcd.drawRect(15,y,18,18,TFT_WHITE);
    String txt=tasks[i];
    if(txt.length()>22) txt=txt.substring(0,22)+"...";
    lcd.drawString(txt,60,y);
  }
}

// ===================== TASK DETAIL =====================
void drawTaskDetail(){
  lcd.fillScreen(TFT_BLACK);
  drawHeader("Task");

  // 🛡️ SAFETY CHECK
  if(selectedTask < 0 || selectedTask >= taskCount){
    lcd.setFont(&fonts::FreeSansBold12pt7b);
    lcd.setTextColor(TFT_DARKGREY);
    lcd.setTextDatum(middle_center);
    lcd.drawString("No task selected", W/2, H/2);
    return;
  }

  lcd.setFont(&fonts::FreeSansBold12pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextWrap(true,true);
  lcd.setCursor(20, HEADER_H + 20);
  lcd.print(tasks[selectedTask]);
  lcd.setTextWrap(false,false);
}



// ===================== MEDICINES =====================
void drawMedicines(){
  lcd.fillScreen(TFT_BLACK);
  drawHeader("Medicines");

  lcd.setFont(&fonts::FreeSansBold12pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(top_left);

  if(medCount==0){
    lcd.drawString("No medicine yet",80,120);
    return;
  }

  for(int i=0;i<medCount;i++){
    int y=HEADER_H+15+i*35;
    lcd.drawRect(15,y,18,18,TFT_WHITE);

    if(meds[i].taken){
      lcd.drawLine(15,y,33,y+18,TFT_GREEN);
      lcd.drawLine(33,y,15,y+18,TFT_GREEN);
    }

    char buf[20];
    sprintf(buf,"%02d:%02d %s",meds[i].hour,meds[i].minute,meds[i].pm?"PM":"AM");
    lcd.drawString(meds[i].name+" "+buf,60,y);
  }
}
void drawLockScreen(){

  lcd.fillScreen(TFT_BLACK);

  // ===== TIME =====
  lcd.setFont(&fonts::FreeSansBold24pt7b);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextDatum(middle_center);

  if(timeValid){
    char buf[16];
    strftime(buf, sizeof(buf), "%I:%M %p", &timeinfo);
    lcd.drawString(buf, W/2, 60);
  }

  // ===== ROOM TEMPERATURE (DHT GPIO 42) =====
  lcd.setFont(&fonts::FreeSansBold12pt7b);

  if(!isnan(roomTemp)){
    lcd.setTextColor(TFT_CYAN);
    lcd.drawString(
      String(roomTemp,1) + "*C  Room",
      W/2,
      105
    );
  }else{
    lcd.setTextColor(TFT_DARKGREY);
    lcd.drawString("Reading temp...", W/2, 105);
  }

  // ===== SLIDER BAR =====
  int barX = 40;
  int barY = 150;
  int barW = 240;
  int barH = 30;

  lcd.setTextColor(TFT_WHITE);
  lcd.drawRoundRect(barX, barY, barW, barH, 15, TFT_WHITE);

  // ===== SLIDER KNOB =====
  lcd.fillRoundRect(
    sliderX,
    barY + 5,
    40,
    barH - 10,
    10,
    TFT_WHITE
  );

  // ===== SLIDER TEXT =====
  lcd.setFont(&fonts::FreeSansBold12pt7b);
  lcd.setTextColor(TFT_DARKGREY);
  lcd.drawString("Slide to Unlock >", W/2, barY + 50);
}



// ===================== UI ROUTER =====================
void drawUI(){

// ===== TIMER DONE HAS HIGHEST PRIORITY =====
if(timerDone){
  drawTimerDone();
  return;
}

// ===== LOCK SCREEN =====
if(isLocked){
  drawLockScreen();

  // 🔔 Medicine popup lock screen par bhi dikhe
  if(medPopup){
    lcd.fillRoundRect(50,80,220,80,16,TFT_BLACK);
    lcd.drawRoundRect(50,80,220,80,16,TFT_GREEN);
    lcd.setFont(&fonts::FreeSansBold12pt7b);
    lcd.setTextColor(TFT_GREEN);
    lcd.setTextDatum(middle_center);
    lcd.drawString("MEDICINE TIME",W/2,105);
    lcd.drawString(popupMedName,W/2,135);
  }

  return;
}

  // ===== NORMAL UI =====
  switch(currentScreen){
    case SCREEN_MAIN: drawMain(); break;
    case SCREEN_TIMER: drawTimer(); break;
    case SCREEN_TASKS: drawTasks(); break;
    case SCREEN_TASK_DETAIL: drawTaskDetail(); break;
    case SCREEN_MEDICINES: drawMedicines(); break;
  }

  // ===== MEDICINE POPUP (UNLOCKED) =====
  if(medPopup){
    lcd.fillRoundRect(50,80,220,80,16,TFT_BLACK);
    lcd.drawRoundRect(50,80,220,80,16,TFT_GREEN);
    lcd.setFont(&fonts::FreeSansBold12pt7b);
    lcd.setTextColor(TFT_GREEN);
    lcd.setTextDatum(middle_center);
    lcd.drawString("MEDICINE TIME",W/2,105);
    lcd.drawString(popupMedName,W/2,135);
  }
}





// ===================== TOUCH =====================
void handleTouch(uint16_t x, uint16_t y){

  // 🔥 SINGLE idle timer
  lastInteraction = millis();

  // ================= LOCK SCREEN =================
  if(isLocked){

    if(y > 150 && y < 180){

      if(swipeStartX == -1){
        swipeStartX = x;
        sliderX = sliderStartX;
        swipeStartTime = millis();
        uiDirty = true;
        return;
      }

      int dx = x - swipeStartX;
      sliderX = sliderStartX + dx;

      if(sliderX < sliderStartX) sliderX = sliderStartX;
      if(sliderX > sliderEndX)   sliderX = sliderEndX;

      if(sliderX >= sliderEndX - 5){
        isLocked = false;
        sliderX = sliderStartX;
        swipeStartX = -1;
        uiDirty = true;
        return;
      }

      uiDirty = true;
      return;
    }
    return;
  }

  // ================= HEADER =================
  // ================= HEADER =================
if(y < HEADER_H){

  // ⬅️ LEFT ARROW
  if(x < 25){

    switch(currentScreen){
      case SCREEN_TASK_DETAIL:
        currentScreen = SCREEN_TASKS;      // 🔥 only exit
        break;

      case SCREEN_MAIN:
        currentScreen = SCREEN_MEDICINES;
        break;

      case SCREEN_TIMER:
        currentScreen = SCREEN_MAIN;
        break;

      case SCREEN_TASKS:
        currentScreen = SCREEN_TIMER;
        break;

      case SCREEN_MEDICINES:
        currentScreen = SCREEN_TASKS;
        break;
    }

    fanPopup = false;    // 🔒 safety
    uiDirty = true;
    return;
  }

  // ➡️ RIGHT ARROW
  if(x > W - 25){

    switch(currentScreen){
      case SCREEN_TASK_DETAIL:
        currentScreen = SCREEN_TASKS;      // 🔥 only exit
        break;

      case SCREEN_MAIN:
        currentScreen = SCREEN_TIMER;
        break;

      case SCREEN_TIMER:
        currentScreen = SCREEN_TASKS;
        break;

      case SCREEN_TASKS:
        currentScreen = SCREEN_MEDICINES;
        break;

      case SCREEN_MEDICINES:
        currentScreen = SCREEN_MAIN;
        break;
    }

    fanPopup = false;    // 🔒 safety
    uiDirty = true;
    return;
  }

  return;
}



  // ================= MAIN SCREEN =================
if(currentScreen == SCREEN_MAIN){

  // 🔥 FAN POPUP ACTIVE
  if(fanPopup){

    if(x < 120 && fanSpeed > 1){
      fanSpeed--;
      updateFanPWM();        // 🔥 SPEED ↓ → PWM
    }

    if(x > 200 && fanSpeed < 5){
      fanSpeed++;
      updateFanPWM();        // 🔥 SPEED ↑ → PWM
    }

    fanOn = true;
    lastInteraction = millis();   // keep popup alive
    uiDirty = true;
    return;
  }

  bool lightChanged = false;

  if(x < 150 && y < 110){
    light1 = !light1;
    lightChanged = true;
  }
  else if(x > 170 && y < 110){
    light2 = !light2;
    lightChanged = true;
  }
  else if(x < 150 && y > 135){
    light3 = !light3;
    lightChanged = true;
  }
  else if(x > 170 && y > 135){

    // 🔥 DOUBLE TAP → FAN POPUP
    if(millis() - lastTapTime < 350){
      fanPopup = true;
      lastInteraction = millis();
      uiDirty = true;
      return;
    }

    // 🔥 SINGLE TAP → FAN ON / OFF
    fanOn = !fanOn;
    updateFanPWM();          // 🔥 FAN TOGGLE → PWM
  }

  if(lightChanged){
    syncGPIO();
  }

  lastTapTime = millis();
  uiDirty = true;
  return;
}


  // ================= TIMER =================
  if(currentScreen == SCREEN_TIMER){
    if(y > 70 && y < 120){
      if(x < 140) selectedField = 0;
      else if(x < 180) selectedField = 1;
      else selectedField = 2;
      uiDirty = true; return;
    }

    if(x > 40 && x < 90 && y > 160 && y < 200){
      if(selectedField == 0 && t_h > 0) t_h--;
      if(selectedField == 1 && t_m > 0) t_m--;
      if(selectedField == 2 && t_s > 0) t_s--;
      uiDirty = true; return;
    }

    if(x > 230 && x < 280 && y > 160 && y < 200){
      if(selectedField == 0 && t_h < 23) t_h++;
      if(selectedField == 1){ t_m++; if(t_m > 59){ t_m = 0; t_h++; }}
      if(selectedField == 2){ t_s++; if(t_s > 59){ t_s = 0; t_m++; }}
      uiDirty = true; return;
    }

    if(x > 135 && x < 185 && y > 160 && y < 200){
      if(timerRunning) timerRunning = false;
      else if(t_h + t_m + t_s > 0){
        timerRunning = true;
        lastTick = millis();
      }
      uiDirty = true; return;
    }
  }

  // ================= TASKS =================
  if(currentScreen == SCREEN_TASKS){
    for(int i = 0; i < taskCount; i++){
      int y0 = HEADER_H + 15 + i * 35;

      if(x > 15 && x < 33 && y > y0 && y < y0 + 18){
        for(int j = i; j < taskCount - 1; j++)
          tasks[j] = tasks[j + 1];
        taskCount--;
        uiDirty = true; return;
      }

      if(x > 60 && y > y0 && y < y0 + 25){
        selectedTask = i;
        currentScreen = SCREEN_TASK_DETAIL;
        taskDetailMillis = millis();
        uiDirty = true; return;
      }
    }
  }

  // ================= MEDICINES =================
  if(currentScreen == SCREEN_MEDICINES){
    for(int i = 0; i < medCount; i++){
      int y0 = HEADER_H + 15 + i * 35;
      if(x > 15 && x < 33 && y > y0 && y < y0 + 18){
        meds[i].taken = true;
        uiDirty = true; return;
      }
    }
  }
}
void updateFanPWM(){
  if(!fanOn){
    ledcWrite(FAN_PWM_PIN, 0);
    return;
  }

  int duty = 0;

  switch(fanSpeed){
    case 1: duty = 51;  break;   // 20%
    case 2: duty = 102; break;   // 40%
    case 3: duty = 153; break;   // 60%
    case 4: duty = 204; break;   // 80%
    case 5: duty = 255; break;   // 100%
    default: duty = 0;  break;
  }

  ledcWrite(FAN_PWM_PIN, duty);
}


void playTimerBeepTune(){
  // 5 short beeps: tin tin tin tin tin
  for(int i = 0; i < 5; i++){
    digitalWrite(BUZZER_PIN, HIGH);
    delay(120);
    digitalWrite(BUZZER_PIN, LOW);
    delay(120);
  }
}



// ===================== WEB =====================
String statusJSON(){
  String json = "{";

  // ----- LIGHTS & FAN -----
  json += "\"light1\":" + String(light1) + ",";
  json += "\"light2\":" + String(light2) + ",";
  json += "\"light3\":" + String(light3) + ",";
  json += "\"fanOn\":"  + String(fanOn)  + ",";
  json += "\"fanSpeed\":" + String(fanSpeed) + ",";

  // ----- TASKS -----
  json += "\"tasks\":[";
  for(int i = 0; i < taskCount; i++){
    json += "\"" + tasks[i] + "\"";
    if(i < taskCount - 1) json += ",";
  }
  json += "],";

  // ----- MEDICINES -----
  json += "\"meds\":[";
  for(int i = 0; i < medCount; i++){
    json += "{";
    json += "\"name\":\"" + meds[i].name + "\",";
    json += "\"hour\":" + String(meds[i].hour) + ",";
    json += "\"minute\":" + String(meds[i].minute) + ",";
    json += "\"pm\":" + String(meds[i].pm);
    json += "}";
    if(i < medCount - 1) json += ",";
  }
  json += "]";

  json += "}";
  return json;
}

String webPage(){
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name=viewport content="width=device-width">
<style>
body{
  background:#000;
  color:#fff;
  font-family:Arial;
  text-align:center;
}
button,select,input{
  width:85%;
  padding:14px;
  margin:6px;
  font-size:18px;
  border:none;
  border-radius:10px;
  background:#222;
  color:#fff;
}
.on{background:#1e88e5}
input{background:#111}
.listItem{
  display:flex;
  justify-content:space-between;
  align-items:center;
  background:#111;
  margin:6px;
  padding:10px;
  border-radius:8px;
}
.delBtn{
  background:#b71c1c;
  width:auto;
  padding:6px 12px;
}
</style>
</head>

<body>

<h2>HomePod</h2>

<!-- ===== CONTROLS ===== -->
<button id="l1" onclick="fetch('/l1')">Light 1</button>
<button id="l2" onclick="fetch('/l2')">Light 2</button>
<button id="l3" onclick="fetch('/l3')">Light 3</button>
<button id="fan" onclick="fetch('/fan')">Fan</button>

<select id="speed" onchange="fetch('/speed?val='+this.value)">
  <option>1</option><option>2</option><option>3</option><option>4</option><option>5</option>
</select>

<!-- ===== TASK ADD ===== -->
<h3>Add Task</h3>
<input id="t" placeholder="Enter task">
<button onclick="fetch('/add?task='+t.value); t.value=''">Add</button>

<h3>To-Do List</h3>
<div id="taskList">No tasks</div>

<!-- ===== MEDICINE ADD ===== -->
<h3>Add Medicine</h3>
<input id="m" placeholder="Medicine name">
<input id="h" placeholder="HH">
<input id="min" placeholder="MM">
<select id="ampm"><option>AM</option><option>PM</option></select>
<button onclick="
fetch('/addmed?name='+m.value+'&h='+h.value+'&m='+min.value+'&p='+ampm.value);
m.value=''; h.value=''; min.value='';
">Add</button>

<h3>Medicine List</h3>
<div id="medList">No medicines</div>

<script>
function renderLists(d){

  // ----- TASKS -----
  let tHTML = "";
  if(d.tasks.length === 0){
    tHTML = "No tasks";
  }else{
    d.tasks.forEach((task,i)=>{
      tHTML += `
        <div class="listItem">
          <span>${task}</span>
          <button class="delBtn" onclick="fetch('/deltask?id=${i}')">D</button>
        </div>`;
    });
  }
  document.getElementById("taskList").innerHTML = tHTML;

  // ----- MEDICINES -----
  let mHTML = "";
  if(d.meds.length === 0){
    mHTML = "No medicines";
  }else{
    d.meds.forEach((med,i)=>{
      let time = med.hour + ":" +
                 (med.minute < 10 ? "0" : "") + med.minute +
                 (med.pm ? " PM" : " AM");

      mHTML += `
        <div class="listItem">
          <span>${med.name} (${time})</span>
          <button class="delBtn" onclick="fetch('/delmed?id=${i}')">D</button>
        </div>`;
    });
  }
  document.getElementById("medList").innerHTML = mHTML;
}

// ===== AUTO REFRESH =====
setInterval(()=>{
  fetch('/status').then(r=>r.json()).then(d=>{
    l1.className = d.light1 ? 'on' : '';
    l2.className = d.light2 ? 'on' : '';
    l3.className = d.light3 ? 'on' : '';
    fan.className = d.fanOn ? 'on' : '';
    speed.value = d.fanSpeed;

    renderLists(d);
  });
},1000);
</script>

</body>
</html>
)rawliteral";
}


void setupWeb(){

  server.on("/", [](){
    server.send(200, "text/html", webPage());
  });

  server.on("/status", [](){
    server.send(200, "application/json", statusJSON());
  });

  // ===== LIGHT 1 =====
  server.on("/l1", [](){
    light1 = !light1;
    syncGPIO();
    uiDirty = true;
    server.send(200, "OK");
  });

  // ===== LIGHT 2 =====
  server.on("/l2", [](){
    light2 = !light2;
    syncGPIO();
    uiDirty = true;
    server.send(200, "OK");
  });

  // ===== LIGHT 3 =====
  server.on("/l3", [](){
    light3 = !light3;
    syncGPIO();
    uiDirty = true;
    server.send(200, "OK");
  });

  // ===== FAN =====
  server.on("/fan", [](){
    fanOn = !fanOn;
    updateFanPWM();
    uiDirty = true;
    server.send(200, "OK");
  });

  // ===== FAN SPEED =====
  server.on("/speed", [](){
    fanSpeed = server.arg("val").toInt();
    fanOn = true;
    updateFanPWM();
    uiDirty = true;
    server.send(200, "OK");
  });

  // ===== ADD TASK =====
  server.on("/add", [](){
    if(taskCount < MAX_TASKS){
      tasks[taskCount++] = server.arg("task");
      uiDirty = true;
    }
    server.send(200, "OK");
  });

  // ===== DELETE TASK =====
  server.on("/deltask", [](){
    int id = server.arg("id").toInt();
    if(id >= 0 && id < taskCount){
      for(int i = id; i < taskCount - 1; i++){
        tasks[i] = tasks[i + 1];
      }
      taskCount--;
      uiDirty = true;
    }
    server.send(200, "OK");
  });

  // ===== ADD MEDICINE =====
  server.on("/addmed", [](){
    if(medCount < MAX_MED){
      meds[medCount].name   = server.arg("name");
      meds[medCount].hour   = server.arg("h").toInt();
      meds[medCount].minute = server.arg("m").toInt();
      meds[medCount].pm     = (server.arg("p") == "PM");
      meds[medCount].taken  = false;
      medCount++;
      uiDirty = true;
    }
    server.send(200, "OK");
  });

  // ===== DELETE MEDICINE =====
  server.on("/delmed", [](){
    int id = server.arg("id").toInt();
    if(id >= 0 && id < medCount){
      for(int i = id; i < medCount - 1; i++){
        meds[i] = meds[i + 1];
      }
      medCount--;
      uiDirty = true;
    }
    server.send(200, "OK");
  });

  server.begin();
}



// ===================== SETUP =====================
void setup(){
  // ===== FAN PWM INIT =====
  ledcAttach(FAN_PWM_PIN, FAN_PWM_FREQ, FAN_PWM_RES);
  ledcWrite(FAN_PWM_PIN, 0);   // fan OFF initially

  // ================= SERIAL =================
  Serial.begin(115200);
  delay(300);

  // ================= DISPLAY =================
  lcd.init();
  lcd.setBrightness(255);
  bootAnimation();

  // ================= WIFI + TIME =================
  Serial.println("[INIT] Connecting WiFi + NTP...");
  initTime();

  if(timeValid){
    Serial.println("[INIT] Time OK");
  }else{
    Serial.println("[INIT] Time FAILED");
  }

  // 🌐 WEB UI IP
  if(WiFi.status() == WL_CONNECTED){
    Serial.print("🌐 Web UI: http://");
    Serial.println(WiFi.localIP());
  }

  // ================= DHT =================
  dht.begin();
  Serial.println("[DHT] Sensor started on GPIO 42");

  // ================= BUZZER =================
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // ================= GPIO OUTPUTS =================
  pinMode(GPIO_LIGHT1, OUTPUT);
  pinMode(GPIO_LIGHT2, OUTPUT);
  pinMode(GPIO_LIGHT3, OUTPUT);

  digitalWrite(GPIO_LIGHT1, HIGH);
  digitalWrite(GPIO_LIGHT2, HIGH);
  digitalWrite(GPIO_LIGHT3, HIGH);

  // ================= WEB SERVER =================
  setupWeb();

  // ================= VOICE MODULE =================
  Serial1.begin(9600, SERIAL_8N1, ASR_RX, ASR_TX);

  while(!asr.begin()){
    Serial.println("DF2301Q retry...");
    delay(2000);
  }

  asr.settingCMD(DF2301Q_UART_MSG_CMD_SET_ENTERWAKEUP, 0);
  asr.settingCMD(DF2301Q_UART_MSG_CMD_SET_MUTE, 0);
  asr.settingCMD(DF2301Q_UART_MSG_CMD_SET_VOLUME, 7);

  // ================= FINAL DRAW =================
  uiDirty = true;
  drawUI();

  Serial.println("✅ SYSTEM READY");
}

// ===================== LOOP =====================
void loop(){
  uint16_t x, y;
  server.handleClient();

  // ===== VOICE =====
  uint8_t cmd = asr.getCMDID();
  if(cmd){
    handleVoiceCommand(cmd);
    lastUserInteraction = millis();   // voice = user activity
    lastInteraction = millis();       // fan popup timer
  }

  // ===== AUTO LOCK AFTER 10s =====
  if(!isLocked && millis() - lastUserInteraction > 10000){
    isLocked = true;
    uiDirty = true;
  }

  // ===== TIME UPDATE =====
  if(timeValid && millis() - lastTimeUpdate > 30000){
    lastTimeUpdate = millis();
    getLocalTime(&timeinfo);
    uiDirty = true;
  }

  // ===== 🌡️ DHT TEMPERATURE READ =====
  if(millis() - lastDHTRead > 3000){
    lastDHTRead = millis();
    float t = dht.readTemperature();
    if(!isnan(t)){
      roomTemp = t;
      uiDirty = true;
    }
  }

  // ===== ⏱️ MEDICINE TIME CHECK =====
  if(timeValid && millis() - lastMedTrigger > 3000){
    lastMedTrigger = millis();

    for(int i = 0; i < medCount; i++){
      if(!meds[i].taken &&
         timeinfo.tm_min == meds[i].minute &&
         ((timeinfo.tm_hour >= 12) == meds[i].pm)){

        if(!medPopup){
          popupMedName = meds[i].name;
          medPopup = true;
          popupMillis = millis();
          medBeepPlayed = false;   // 🔔 allow beep
          uiDirty = true;
        }
      }
    }
  }

  // ===== 🔔 MEDICINE BEEP =====
  if(medPopup && !medBeepPlayed){
    playTimerBeepTune();
    medBeepPlayed = true;
    uiDirty = true;
  }

  // ===== MEDICINE POPUP AUTO CLOSE =====
  if(medPopup && millis() - popupMillis > 5000){
    medPopup = false;
    medBeepPlayed = false;
    digitalWrite(BUZZER_PIN, LOW);
    uiDirty = true;
  }

  // ===== TIMER COUNTDOWN =====
  if(timerRunning && millis() - lastTick >= 1000){
    lastTick = millis();

    if(t_s > 0) t_s--;
    else if(t_m > 0){ t_m--; t_s = 59; }
    else if(t_h > 0){ t_h--; t_m = 59; t_s = 59; }
    else{
      timerRunning = false;
      timerDone = true;
      timerDoneMillis = millis();
      timerBeepPlayed = false;
    }
    uiDirty = true;
  }

  // ===== TIMER DONE → BEEP =====
  if(timerDone && !timerBeepPlayed){
    playTimerBeepTune();
    timerBeepPlayed = true;
    uiDirty = true;
  }

  // ===== TIMER DONE RESET =====
  if(timerDone && millis() - timerDoneMillis > 8000){
    timerDone = false;
    t_h = t_m = t_s = 0;
    timerBeepPlayed = false;
    digitalWrite(BUZZER_PIN, LOW);
    uiDirty = true;
  }

  // ===== 🔥 FAN POPUP AUTO CLOSE =====
  if(fanPopup && millis() - lastInteraction > 2500){
    fanPopup = false;
    uiDirty = true;
  }

  // ===== TOUCH =====
  if(lcd.getTouch(&x, &y)){
    lastUserInteraction = millis();
    lastInteraction = millis();

    if(isLocked){
      handleTouch(x, y);
    }
    else if(!touchLock){
      touchLock = true;
      handleTouch(x, y);
    }
  }else{
    touchLock = false;
    swipeStartX = -1;
    sliderX = sliderStartX;
  }

  // ===== DRAW UI =====
  if(uiDirty){
    drawUI();
    uiDirty = false;
  }

  delay(8);
}

