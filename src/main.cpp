#include <Arduino.h>
#include <WiFi.h>                                             // Wifi制御用ライブラリ
#include <stdio.h>                                            // 標準入出力処理ライブラリ

// ------------------------------------------------------------
// 定数/変数　定義部　Constant / variable definition section.
// ------------------------------------------------------------ 
const char* ssid = "";                               // ご自分のWi-FiルータのSSIDを記述します。 "your-ssid"
const char* password = "";                       // ご自分のWi-Fiルータのパスワードを記述します。"your-password"

const int RED = 16;
const int RED2 = 17;
const int YELLOW = 0;
const int YELLOW2 = 1;
const int GREEN = 2;
const int GREEN2 = 3;
const int SOUND_FAST = 4;
const int SOUND_SLOW = 5;

int port = 80;                                                // Http接続に使うポート番号の設定

WiFiServer server(port);                                      // WiFiサーバーの生成

const long TIMEOUT_TIME = 2000;                               // タイムアウト時間の定義（2s=2000ms) Definition of timeout time (2s = 2000ms).
String Header;                                                // HTTPリクエストの格納用 For storing HTTP requests
unsigned long CurrentTime = millis();                         // 現在時刻格納用 For storing the current time.
unsigned long PreviousTime = 0;                               // 前の時刻格納用 For storing the previous time.

// ------------------------------------------------------------
// Webリクエスト応答関数
// Run_WEB()
// ------------------------------------------------------------
void Run_WEB() {
  WiFiClient client = server.available();                     // Listen for incoming clients

  if (client) {                                               // 新しいクライアント接続がある場合に以下を実行
    CurrentTime = millis();                                   // 現在時刻の取得
    PreviousTime = CurrentTime;                               // 前回時刻への現在時刻の格納

    // for debug
    // Serial.println("New Client.");                         // デバッグ用シリアルに表示

    String _CurrentLine = "";                                 // クライアントからの受信データを保持する文字列の生成
    while (client.connected() && CurrentTime - PreviousTime <= TIMEOUT_TIME) {  // クライアントが接続している間繰り返し実行する
      CurrentTime = millis();                                 // 現在時刻の更新
      if (client.available()) {                               // クライアントから読み取る情報（バイト）がある場合
        char _c = client.read();                              // 値（バイト）を読み取り

        // for debug
        // Serial.write(_c);                                  // デバッグ用シリアルに表示

        Header += _c;                                         // リクエストに値（バイト）を格納
        if (_c == '\n') {                                     // 値が改行文字の場合に以下を実行
          //現在の行が空白の場合、2つの改行文字が連続して表示
          //クライアントのHTTPリクエストは終了であるため、応答を送信
          if (_CurrentLine.length() == 0) {
            int pos1 = 0;
            int pos2 = 0;

            // Header String から各閾値を取得し、変数に格納
            pos1 = Header.indexOf("massaget=", pos2);
            pos2 = Header.indexOf(" ", pos1);
            
            // 受信したメッセージをシリアルモニタに表示する
            String mes_str =  Header.substring(pos1 + 9, pos2); // 受信したテキストからメッセージを抽出
            mes_str.replace("+", " ");
            if (mes_str.length() <= 32) {                       // テキストの長さがcharの大きさよりも小さい場合に実行
              char buf[64];
              char mes_text[32];
              mes_str.toCharArray(mes_text, 32);
              sprintf(buf, "Send Message : %s", mes_text);
              Serial.println(buf);
            }
          
            // HTTPヘッダーは常に応答コードで始まる（例：HTTP/1.1 200 OK)
            // Content-typeでクライアントが何が来るのか知ることができその後空白行になる
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // HTML Webページの表示
            client.println("<!DOCTYPE html><html>");
            client.println("<html lang=\"ja\">");
            client.println("<head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>KARAKURI-MUSHA -Http connect test- </title>");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // CSSのスタイルの指定
            client.println("<style>");
            client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".btn_1 {background-color: #e64e20; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 15px; margin: 2px; cursor: pointer;border-radius: 100vh;width: 150px;}");
            client.println("</style></head>");

            // Webページ本体(タイトルと説明)
            client.println("<body><h3>KARAKURI-MUSHA</h3>");
            client.println("<h3>- Http 接続テスト-</h3>");
            client.println("<form action=\"MESSAGEQ\" name=\"massageq\" method=\"get\">");
            client.println("<input type=\"text\" name=\"massaget\">");
            client.println("<p ><input type=\"submit\" value=\"SEND\" class=\"btn_1\"></a></p></form>");

            // 本文末
            client.println("</body></html>");
            // HTTP応答は別の空白行で終了
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            _CurrentLine = "";
          }
        } else if (_c != '\r') {  // if you got anything else but a carriage return character,
          _CurrentLine += _c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    Header = "";
    client.flush();
  }
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

  

  // シリアルコンソールの開始　Start serial console.
  Serial.begin(9600);
  delay(3000);

 // WiFi.mode(WIFI_STA);
  Serial.print("Connecting to '");
  Serial.print(ssid);
  Serial.println("'");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
    Serial.println("\nConnected to WiFi\n\nConnect to server at %s:%d\n");
    Serial.print(WiFi.localIP().toString().c_str());
    Serial.print(port);

  server.begin();
}
void loop() {
  Run_WEB();
}