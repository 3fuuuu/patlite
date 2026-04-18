#include <Arduino.h>
#include <WiFi.h>
#include <stdio.h>

const char* ssid = "your-ssid";
const char* password = "your-password";

const int RED = 16;
const int RED2 = 17;
const int YELLOW = 0;
const int YELLOW2 = 1;
const int GREEN = 2;
const int GREEN2 = 3;
const int SOUND_FAST = 4;
const int SOUND_SLOW = 5;

WiFiServer server(80);

const long TIMEOUT_TIME = 2000;
String Header;
unsigned long CurrentTime = millis();
unsigned long PreviousTime = 0;

bool stRED=false, stRED2=false, stYELLOW=false, stYELLOW2=false;
bool stGREEN=false, stGREEN2=false, stSFAST=false, stSSLOW=false;

void applyOutputs() {
  digitalWrite(RED, stRED ? HIGH : LOW);
  digitalWrite(RED2, stRED2 ? HIGH : LOW);
  digitalWrite(YELLOW, stYELLOW ? HIGH : LOW);
  digitalWrite(YELLOW2, stYELLOW2 ? HIGH : LOW);
  digitalWrite(GREEN, stGREEN ? HIGH : LOW);
  digitalWrite(GREEN2, stGREEN2 ? HIGH : LOW);
  digitalWrite(SOUND_FAST, stSFAST ? HIGH : LOW);
  digitalWrite(SOUND_SLOW, stSSLOW ? HIGH : LOW);
}

const char HTML_PAGE[] = R"HTML(
<!DOCTYPE html><html lang="jp"><head>
  <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>REMOTE-CONTROLLER</title>
  <style>
    body{font-family:sans-serif;background:#fff;max-width:520px;margin:0 auto;text-align:center;}
    h1{color:#333;font-size:26px;margin:12px auto;}
    .row{display:flex;justify-content:center;gap:10px;margin:10px 0;flex-wrap:wrap;}
    .btn{height:56px;min-width:120px;color:#2c2b2b;background:#dddde9;font-size:16px;font-weight:bold;border-radius:10px;border:0;}
    .title{min-width:120px;font-weight:bold}
    table{margin:10px auto;border-collapse:collapse}
    td{border:1px solid #999;padding:8px 10px}
  </style></head><body>
    <h1>patliteぴかぴか</h1>

    <div class="row"><div class="title">RED</div>
      <button class="btn" onclick="send('red_on')">ON</button>
      <button class="btn" onclick="send('red_off')">OFF</button>
      <div id="st_red">?</div>
    </div>

    <div class="row"><div class="title">RED2</div>
      <button class="btn" onclick="send('red2_on')">ON</button>
      <button class="btn" onclick="send('red2_off')">OFF</button>
      <div id="st_red2">?</div>
    </div>

    <div class="row"><div class="title">YELLOW</div>
      <button class="btn" onclick="send('yellow_on')">ON</button>
      <button class="btn" onclick="send('yellow_off')">OFF</button>
      <div id="st_yellow">?</div>
    </div>

    <div class="row"><div class="title">YELLOW2</div>
      <button class="btn" onclick="send('yellow2_on')">ON</button>
      <button class="btn" onclick="send('yellow2_off')">OFF</button>
      <div id="st_yellow2">?</div>
    </div>

    <div class="row"><div class="title">GREEN</div>
      <button class="btn" onclick="send('green_on')">ON</button>
      <button class="btn" onclick="send('green_off')">OFF</button>
      <div id="st_green">?</div>
    </div>

    <div class="row"><div class="title">GREEN2</div>
      <button class="btn" onclick="send('green2_on')">ON</button>
      <button class="btn" onclick="send('green2_off')">OFF</button>
      <div id="st_green2">?</div>
    </div>

    <div class="row"><div class="title">SOUND_FAST</div>
      <button class="btn" onclick="send('sfast_on')">ON</button>
      <button class="btn" onclick="send('sfast_off')">OFF</button>
      <div id="st_sfast">?</div>
    </div>

    <div class="row"><div class="title">SOUND_SLOW</div>
      <button class="btn" onclick="send('sslow_on')">ON</button>
      <button class="btn" onclick="send('sslow_off')">OFF</button>
      <div id="st_sslow">?</div>
    </div>

    <div class="row">
      <button class="btn" onclick="send('all_off')">ALL OFF</button>
    </div>

    <table>
      <tr><td>状態(まとめ)</td><td><pre id="all"></pre></td></tr>
    </table>

  <script type="text/javascript">
    async function send(cmd){
      try{ await fetch("/?cmd=" + cmd); }catch(e){ console.log(e); }
    }

    async function updateStatus(){
      try{
        const r = await fetch("/?cmd=status");
        if(!r.ok) return;
        const t = await r.text(); // 例: red=1&red2=0&...
        document.getElementById("all").textContent = t;

        document.getElementById("st_red").textContent     = t.includes("red=1") ? "ON" : "OFF";
        document.getElementById("st_red2").textContent    = t.includes("red2=1") ? "ON" : "OFF";
        document.getElementById("st_yellow").textContent  = t.includes("yellow=1") ? "ON" : "OFF";
        document.getElementById("st_yellow2").textContent = t.includes("yellow2=1") ? "ON" : "OFF";
        document.getElementById("st_green").textContent   = t.includes("green=1") ? "ON" : "OFF";
        document.getElementById("st_green2").textContent  = t.includes("green2=1") ? "ON" : "OFF";
        document.getElementById("st_sfast").textContent   = t.includes("sfast=1") ? "ON" : "OFF";
        document.getElementById("st_sslow").textContent   = t.includes("sslow=1") ? "ON" : "OFF";
      }catch(e){ console.log(e); }
    }

    setInterval(updateStatus, 1000);
    updateStatus();
  </script></body></html>
)HTML";

void Run_WEB() {
  WiFiClient client = server.available();
  if (!client) return;

  CurrentTime = millis();
  PreviousTime = CurrentTime;

  String currentLine = "";
  Header = "";

  while (client.connected() && CurrentTime - PreviousTime <= TIMEOUT_TIME) {
    CurrentTime = millis();
    if (!client.available()) continue;

    char c = client.read();
    Header += c;

    if (c == '\n') {
      if (currentLine.length() == 0) {
        // ===== ここでコマンド判定（初心者向けにifを並べる） =====
        if (Header.indexOf("GET /?cmd=red_on") >= 0)      stRED = true;
        if (Header.indexOf("GET /?cmd=red_off") >= 0)     stRED = false;

        if (Header.indexOf("GET /?cmd=red2_on") >= 0)     stRED2 = true;
        if (Header.indexOf("GET /?cmd=red2_off") >= 0)    stRED2 = false;

        if (Header.indexOf("GET /?cmd=yellow_on") >= 0)   stYELLOW = true;
        if (Header.indexOf("GET /?cmd=yellow_off") >= 0)  stYELLOW = false;

        if (Header.indexOf("GET /?cmd=yellow2_on") >= 0)  stYELLOW2 = true;
        if (Header.indexOf("GET /?cmd=yellow2_off") >= 0) stYELLOW2 = false;

        if (Header.indexOf("GET /?cmd=green_on") >= 0)    stGREEN = true;
        if (Header.indexOf("GET /?cmd=green_off") >= 0)   stGREEN = false;

        if (Header.indexOf("GET /?cmd=green2_on") >= 0)   stGREEN2 = true;
        if (Header.indexOf("GET /?cmd=green2_off") >= 0)  stGREEN2 = false;

        if (Header.indexOf("GET /?cmd=sfast_on") >= 0)    stSFAST = true;
        if (Header.indexOf("GET /?cmd=sfast_off") >= 0)   stSFAST = false;

        if (Header.indexOf("GET /?cmd=sslow_on") >= 0)    stSSLOW = true;
        if (Header.indexOf("GET /?cmd=sslow_off") >= 0)   stSSLOW = false;

        if (Header.indexOf("GET /?cmd=all_off") >= 0) {
          stRED=stRED2=stYELLOW=stYELLOW2=stGREEN=stGREEN2=stSFAST=stSSLOW=false;
        }

        applyOutputs();
        if (Header.indexOf("GET /?cmd=status") >= 0) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/plain; charset=utf-8");
          client.println("Connection: close");
          client.println();
          client.printf("red=%d&red2=%d&yellow=%d&yellow2=%d&green=%d&green2=%d&sfast=%d&sslow=%d",
                        stRED, stRED2, stYELLOW, stYELLOW2, stGREEN, stGREEN2, stSFAST, stSSLOW);
          client.println();
          client.println();
          break;
        }

        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html; charset=utf-8");
        client.println("Connection: close");
        client.println();
        client.print(HTML_PAGE);
        client.println();
        break;
      } else {
        currentLine = "";
      }
    } else if (c != '\r') {
      currentLine += c;
    }
  }

  Header = "";
  client.flush();
  client.stop();
}

void setup() {
  Serial.begin(9600);
  delay(3000);

  pinMode(RED, OUTPUT);
  pinMode(RED2, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(YELLOW2, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(GREEN2, OUTPUT);
  pinMode(SOUND_FAST, OUTPUT);
  pinMode(SOUND_SLOW, OUTPUT);

  applyOutputs(); // 全部OFF

  Serial.printf("Connecting to '%s' \n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.printf("\nConnected to WiFi\nIP: %s\n",
                WiFi.localIP().toString().c_str());

  server.begin();
}

void loop() {
  Run_WEB();
}