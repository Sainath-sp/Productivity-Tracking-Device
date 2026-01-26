#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <time.h>

/* ===================== DISPLAY ===================== */
Adafruit_SH1106G display(128, 64, &Wire, -1);

enum DisplayState {SHOW_HOME,SHOW_UPLOADING};

DisplayState currentState = SHOW_HOME;
unsigned long uploadingStart = 0;

/* ===================== WIFI ===================== */
const char* ap_ssid = "ESP WATCH";
const char* ap_password = "12345678";
WebServer server(80);

/* ===================== DATA ===================== */
String tasks[4];
bool taskValid[4] = {false, false, false, false};

struct tm rtcTime;
unsigned long lastRTCUpdate = 0;

/* ===================== BITMAPS ===================== */
// ---- uploading icon ----
static const unsigned char PROGMEM update_icon[] = {
  0x3f,0xff,0x80,0x40,0x00,0x40,0x80,0x00,0x20,0x80,0x00,0x20,0x81,
  0x80,0x20,0x81,0xc0,0x20,0x81,0xe0,0x20,0x81,0xf0,0x20,0x81,0xf8,
  0x20,0x81,0xfc,0x20,0x81,0xf8,0x20,0x81,0xf0,0x20,0x81,0xe0,0x20,
  0x81,0xc0,0x20,0x81,0x80,0x20,0x80,0x00,0x20,0x80,0x00,0x20,0xc0,
  0x00,0x60,0x7f,0xff,0xc0,0x3f,0xff,0x80};

static const unsigned char PROGMEM battery_icon[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xff,0xf0,0x80,0x00,0x08,0xb6,0xdb,0x68,0xb6,
  0xdb,0x6e,0xb6,0xdb,0x61,0xb6,0xdb,0x61,0xb6,0xdb,0x61,0xb6,0xdb,0x61,0xb6,0xdb,
  0x61,0xb6,0xdb,0x6e,0xb6,0xdb,0x68,0x80,0x00,0x08,0x7f,0xff,0xf0,0x00,0x00,0x00};


/* ===================== HTML ===================== */
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
<h2>ESP WATCH</h2>

Task 1 <input id="t1"><br><br>
Task 2 <input id="t2"><br><br>
Task 3 <input id="t3"><br><br>
Task 4 <input id="t4"><br><br>

<button onclick="send()">Update</button>

<script>
function send(){
  let now = new Date();
  let payload = {
    time: now.getFullYear()+"-"+(now.getMonth()+1).toString().padStart(2,'0')+"-"+now.getDate().toString().padStart(2,'0')
          +" "+now.getHours().toString().padStart(2,'0')+":"+now.getMinutes().toString().padStart(2,'0')+":"+now.getSeconds().toString().padStart(2,'0'),
    tasks: [
      document.getElementById("t1").value,
      document.getElementById("t2").value,
      document.getElementById("t3").value,
      document.getElementById("t4").value
    ]
  };

  fetch("/update",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify(payload)});
}
</script>
</body>
</html>
)rawliteral";

/* ===================== FUNCTIONS ===================== */

void drawUploading() {
  display.clearDisplay();
  display.drawBitmap(55, 13, update_icon, 19, 20, 1);
  display.setTextColor(1);
  display.setTextWrap(false);
  display.setCursor(26, 40);
  display.print("Updating..!!");
  display.display();
}

void drawHome() {
  display.clearDisplay();

  // Top bar
  display.drawLine(0,17,127,17,1);
  display.drawBitmap(100, 0, battery_icon, 24, 16, 1);
  display.drawLine(43, 19, 43, 64, 1);


  // Day
  char dayChar[2];
  strftime(dayChar, sizeof(dayChar), "%a", &rtcTime); // "Mon", "Tue", etc.
  display.setTextSize(2);
  display.setCursor(4, 1);
  display.print(dayChar[0]);   // Print only first letter

  // Date
  char dateStr[9];
  strftime(dateStr, sizeof(dateStr), "%d-%m-%y", &rtcTime);
  display.setTextSize(1);
  display.setCursor(18,8);
  display.print(dateStr);

  // Time
  display.setTextSize(3);
  display.setCursor(4,20);
  display.printf("%02d", rtcTime.tm_hour);
  display.setCursor(4,43);
  display.printf("%02d", rtcTime.tm_min);

  display.setTextSize(1);

  // Tasks
  int y = 24;
  for(int i=0;i<4;i++){
    if(taskValid[i]){
      display.drawRect(47, y, 7, 7, 1);
      display.setCursor(57,y);
      display.print(tasks[i]);
    }
    y += 10;
  }

  display.display();
}

void updateRTC() {
  if(millis() - lastRTCUpdate >= 1000){
    lastRTCUpdate = millis();
    rtcTime.tm_sec++;
    mktime(&rtcTime);
  }
}

/* ===================== WEB HANDLERS ===================== */

void handleRoot() {
  server.send(200,"text/html",PAGE);
}

void handleUpdate() {
  String body = server.arg("plain");

  // --- show uploading ---
  currentState = SHOW_UPLOADING;
  uploadingStart = millis();

  // --- parse time ---
  int t = body.indexOf("\"time\":\"") + 8;
  String timeStr = body.substring(t, body.indexOf("\"", t));
  sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d",
         &rtcTime.tm_year, &rtcTime.tm_mon, &rtcTime.tm_mday,
         &rtcTime.tm_hour, &rtcTime.tm_min, &rtcTime.tm_sec);
  rtcTime.tm_year -= 1900;
  rtcTime.tm_mon -= 1;
  mktime(&rtcTime);

  // --- parse tasks ---
  for(int i=0;i<4;i++) taskValid[i] = false;

  int pos = body.indexOf("[");
  for(int i=0;i<4;i++){
    int s = body.indexOf("\"", pos+1);
    int e = body.indexOf("\"", s+1);
    if(s<0 || e<0) break;
    String val = body.substring(s+1,e);
    if(val.length()){
      tasks[i] = val;
      taskValid[i] = true;
    }
    pos = e+1;
  }

  server.send(200,"text/plain","OK");
}

/* ===================== SETUP ===================== */
void setup() {
  Serial.begin(115200);

  display.begin(0x3C,true);
  display.clearDisplay();
  display.display();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);

  server.on("/", handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.begin();
}

/* ===================== LOOP ===================== */
void loop() {
  server.handleClient();
  updateRTC();
  
  if(currentState == SHOW_UPLOADING){
    drawUploading();
    if(millis() - uploadingStart > 3000){
      currentState = SHOW_HOME;
    }
  } else {
    drawHome();
  }
}
