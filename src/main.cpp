#include <Arduino.h>
#include <WiFi.h>
#include <stdio.h>

// ------------------------------------------------------------
// Wi-Fi設定
// ------------------------------------------------------------
const char* ssid = "";
const char* password = "";

// ------------------------------------------------------------
// ピン定義
// ------------------------------------------------------------
const int RED        = 16;
const int RED2       = 17;
const int YELLOW     = 0;
const int YELLOW2    = 1;
const int GREEN      = 2;
const int GREEN2     = 3;
const int SOUND_FAST = 4;
const int SOUND_SLOW = 5;

// ------------------------------------------------------------
// サーバ
// ------------------------------------------------------------
WiFiServer server(80);
const uint32_t TIMEOUT_MS = 2000;

// ------------------------------------------------------------
// State（状態だけ持つ）
// ------------------------------------------------------------
struct State {
  bool red = false;
  bool red2 = false;
  bool yellow = false;
  bool yellow2 = false;
  bool green = false;
  bool green2 = false;
  bool sound_fast = false;
  bool sound_slow = false;
};

State st;

static const char* onoff(bool v) { return v ? "ON" : "OFF"; }

void httpState() {
  digitalWrite(RED,        st.red        ? HIGH : LOW);
  digitalWrite(RED2,       st.red2       ? HIGH : LOW);
  digitalWrite(YELLOW,     st.yellow     ? HIGH : LOW);
  digitalWrite(YELLOW2,    st.yellow2    ? HIGH : LOW);
  digitalWrite(GREEN,      st.green      ? HIGH : LOW);
  digitalWrite(GREEN2,     st.green2     ? HIGH : LOW);
  digitalWrite(SOUND_FAST, st.sound_fast ? HIGH : LOW);
  digitalWrite(SOUND_SLOW, st.sound_slow ? HIGH : LOW);
}
void setByKey(const String& header, const char* key, bool& value) {
  String k1 = String(key) + "=1";
  String k0 = String(key) + "=0";
  if (header.indexOf(k1) >= 0) value = true;
  if (header.indexOf(k0) >= 0) value = false;
}

void updateState(const String& header) {
  setByKey(header, "red",        st.red);
  setByKey(header, "red2",       st.red2);
  setByKey(header, "yellow",     st.yellow);
  setByKey(header, "yellow2",    st.yellow2);
  setByKey(header, "green",      st.green);
  setByKey(header, "green2",     st.green2);
  setByKey(header, "sound_fast", st.sound_fast);
  setByKey(header, "sound_slow", st.sound_slow);

  if (header.indexOf("all=0") >= 0) {
    st = State{}; 
  }
}

String makeHtml() {
  String html = R"HTML(
<!doctype html><html lang="ja"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>patlite ぴかぴか</title>
<style>
  body{font-family:Helvetica;max-width:560px;margin:0 auto;text-align:center}
  .card{border:1px solid #ccc;border-radius:10px;padding:10px;margin:12px}
  .row{margin:10px 0}
  a.btn{display:inline-block;min-width:120px;padding:12px 14px;margin:6px;
        background:#1e88e5;color:#fff;text-decoration:none;border-radius:999px;font-weight:bold}
  a.off{background:#616161}
</style></head><body>
<h3>KARAKURI-MUSHA</h3>
<h3>- patlite ぴかぴか -</h3>
)HTML";

  html += "<div class='card'>";
  html += "RED: " + String(onoff(st.red)) + " / RED2: " + String(onoff(st.red2)) + "<br>";
  html += "YELLOW: " + String(onoff(st.yellow)) + " / YELLOW2: " + String(onoff(st.yellow2)) + "<br>";
  html += "GREEN: " + String(onoff(st.green)) + " / GREEN2: " + String(onoff(st.green2)) + "<br>";
  html += "SOUND_FAST: " + String(onoff(st.sound_fast)) + " / SOUND_SLOW: " + String(onoff(st.sound_slow)) + "<br>";
  html += "</div>";

  auto row = [&](const char* label, const char* key){
    html += "<div class='row'><strong>";
    html += label;
    html += "</strong><br>";
    html += "<a class='btn' href='/?";
    html += key;
    html += "=1'>ON</a>";
    html += "<a class='btn off' href='/?";
    html += key;
    html += "=0'>OFF</a></div>";
  };

  row("RED", "red");
  row("RED2", "red2");
  row("YELLOW", "yellow");
  row("YELLOW2", "yellow2");
  row("GREEN", "green");
  row("GREEN2", "green2");
  row("SOUND_FAST", "sound_fast");
  row("SOUND_SLOW", "sound_slow");

  html += "<div class='row'><a class='btn off' href='/?all=0'>ALL OFF</a></div>";
  html += "</body></html>";
  return html;
}

void handleClientOnce() {
  WiFiClient client = server.available();
  if (!client) return;

  uint32_t start = millis();
  String header;
  String line;

  while (client.connected() && (millis() - start) < TIMEOUT_MS) {
    if (!client.available()) continue;

    char c = client.read();
    header += c;

    if (c == '\n') {
      if (line.length() == 0) break; // 空行＝ヘッダ終端
      line = "";
    } else if (c != '\r') {
      line += c;
    }
  }

  updateState(header);

  // レスポンス
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html; charset=utf-8");
  client.println("Connection: close");
  client.println();
  client.print(makeHtml());
  client.println();

  client.stop();
}
void setup() {
  pinMode(RED, OUTPUT);
  pinMode(RED2, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(YELLOW2, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(GREEN2, OUTPUT);
  pinMode(SOUND_FAST, OUTPUT);
  pinMode(SOUND_SLOW, OUTPUT);

  Serial.begin(115200);
  delay(500);

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nConnected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.begin();

  httpState();
}

void loop() {
  handleClientOnce();
  httpState();
}