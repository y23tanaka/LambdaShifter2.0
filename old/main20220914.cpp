///////// Description //////////
/*
  This is a lambda shift device for engines with narrow band sensors.
  Normally, the ECU judges the O2 sensor output of 0.4 to 0.5v as A/F 14.7,
  below 0.4v as lean, and above 0.5v as rich.

  STATUS_LED
  For detecting O2 sensor failure and checking switch settings.
  When the O2 sensor input is 0.1v or less (when the engine is stopped), the following operation is performed.
  If the blinking continues even after the engine is turned on, there is a possibility of 02 sensor failure.
  If the blinking interval does not change even after switching the switch, the switch may be faulty.

  Fuel_LED
  Lights up when the rich signal is sent to the ECU. You can check if it is working properly as a system.
  If the light does not come on when the engine is started and the accelerator pedal is opened, there may be some kind of error.
*/
#include <Arduino.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <WebSerial.h>
#include <AsyncElegantOTA.h>
#define FORMAT_SPIFFS_IF_FAILED true

///// Firm version
const String firm_version = "2.0.2";

///// WiFi Config /////
const char ssid[] = "LS20-AP0000";
const char pass[] = "NJdcTw5gxU"; // パスワードは8文字以上
const IPAddress ip(10, 1, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
AsyncWebServer server(80); // ポート設定

///// Update duration ////
const int update_frequency = 200;

// These are HW configuration.  Don't change this value
const int O2IN_PIN1 = 32;
const int O2IN_PIN2 = 35;
const int O2OUT_PIN1 = 25;
const int O2OUT_PIN2 = 26;
const int CODE_PIN1 = 23;
const int CODE_PIN2 = 13;

String file_version;
String af_value;
String map_value;
String secondary_value = "OFF";
String log_sw = "OFF";
String graph_sw;
float shift_value;
float serial_out_volt1;
float serial_out_volt2;
float in_volt1;
float in_volt2;
int code_value;
int counter;
String processor(const String &var)
{
  if (var == "AF_STATE")
  {
    if (af_value == "0.01")
    {
      return ("Disable");
    }
    else
    {
      return af_value;
    }
  }
  if (var == "MAP_STATE")
  {
    return map_value;
  }
  if (var == "LOG_STATE")
  {
    return log_sw;
  }
  if (var == "SECONDARY_STATE")
  {
    return secondary_value;
  }
  if (var == "FIRM_VIRSION")
  {
    return firm_version;
  }

  if (var == "FILE_VERSION")
  {
    return file_version;
  }

  if (var == "MAP_SELECT1" && map_value == "1")
  {
    return "selected";
  }

  if (var == "MAP_SELECT2" && map_value == "2")
  {
    return "selected";
  }

  if (var == "MAP_SELECT3" && map_value == "3")
  {
    return "selected";
  }
  if (var == "AF_SELECT000" && af_value == "0.00")
  {
    return "selected";
  }
  if (var == "AF_SELECT010" && af_value == "0.10")
  {
    return "selected";
  }

  if (var == "AF_SELECT020" && af_value == "0.20")
  {
    return "selected";
  }

  if (var == "AF_SELECT025" && af_value == "0.25")
  {
    return "selected";
  }

  if (var == "AF_SELECT-010" && af_value == "-0.10")
  {
    return "selected";
  }

  if (var == "AF_SELECT-020" && af_value == "-0.20")
  {
    return "selected";
  }

  if (var == "AF_SELECT-030" && af_value == "-0.30")
  {
    return "selected";
  }

  if (var == "AF_SELECT-040" && af_value == "-0.40")
  {
    return "selected";
  }

  if (var == "AF_SELECT001" && af_value == "0.01")
  {
    return "selected";
  }

  return String();
}

///////////////////////////// Execution Part //////////////////////////////
void setup()
{
  pinMode(O2OUT_PIN1, OUTPUT);
  pinMode(O2OUT_PIN2, OUTPUT);
  pinMode(CODE_PIN1, OUTPUT);
  pinMode(CODE_PIN2, OUTPUT);

  WiFi.softAP(ssid, pass);           // SSIDとパスの設定
  delay(100);                        // このdelayを入れないと失敗する場合がある
  WiFi.softAPConfig(ip, ip, subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定
  IPAddress myIP = WiFi.softAPIP();  // WiFi.softAPIP()でWiFi起動

  // SPIFFSのセットアップ
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  /////ログファイル準備
  SPIFFS.remove("/ls-oat_log.csv");
  File fileToWrite = SPIFFS.open("/ls-oat_log.csv", FILE_WRITE);
  fileToWrite.println("time(mil),in_volt1,out_volt1,in_volt2,out_volt2,shift_value,map_value");
  fileToWrite.close();

  ////保存したシフト値読み取り
  String wrfile1 = "/shift_value";
  File fr1 = SPIFFS.open(wrfile1.c_str(), "r");
  String readStr1 = fr1.readStringUntil('\n');
  af_value = readStr1;
  shift_value = readStr1.toFloat();
  fr1.close();

  ////保存したファイルバージョン読み取り
  String wrfile4 = "/file_version";
  File fr4 = SPIFFS.open(wrfile4.c_str(), "r");
  file_version = fr4.readStringUntil('\n');
  fr4.close();

  ////保存したコード値読み取り、コード値に代入
  String wrfile2 = "/code_value";
  File fr2 = SPIFFS.open(wrfile2.c_str(), "r");
  String readStr2 = fr2.readStringUntil('\n');
  map_value = readStr2;
  code_value = readStr2.toInt();
  fr2.close();
  if (code_value == 1)
  {
    digitalWrite(CODE_PIN1, LOW);
    digitalWrite(CODE_PIN2, LOW);
  }
  if (code_value == 2)
  {
    digitalWrite(CODE_PIN1, HIGH);
    digitalWrite(CODE_PIN2, LOW);
  }
  if (code_value == 3)
  {
    digitalWrite(CODE_PIN1, LOW);
    digitalWrite(CODE_PIN2, HIGH);
  }

  ////保存したセカンダリ値読み取り
/*  String wrfile3 = "/secondary_value";
  File fr3 = SPIFFS.open(wrfile3.c_str(), "r");
  String readStr3 = fr3.readStringUntil('\n');
  secondary_value = readStr3;
  fr3.close();  */

  // GETリクエストに対するハンドラーを登録 rootにアクセスされた時のレスポンス
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/setup.html", String(), false, processor); });

  server.on("/ls_logo", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/ls_logo.png", "image/png"); });

  server.on("/uploader.png", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/uploader.png", "image/png"); });

  server.on("/favicon", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/favicon.ico", "image/x-icon"); });

  //  Log on or  off
  server.on("/log_sw", HTTP_POST, [](AsyncWebServerRequest *request)
            {
             if(log_sw == "OFF"){log_sw = "ON";}
              else{log_sw = "OFF";}    
            request->send(SPIFFS, "/setup.html", String(), false, processor); });

  server.on("/ls-oat_log.csv", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/ls-oat_log.csv", "text/csv"); });

  // AF setting
  server.on("/set_af", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    af_value = request->arg("af_param");
    shift_value = af_value.toFloat();
    String wrfile1 = "/shift_value";
    String writeStr1 = af_value;
    File fw1 = SPIFFS.open(wrfile1.c_str(), "w");
    fw1.println( writeStr1 );
    fw1.close();
    request->send(SPIFFS, "/setup.html", String(), false, processor); });

  //  secondary on or  off
  server.on("/secondary_sw", HTTP_POST, [](AsyncWebServerRequest *request)
            {
if(secondary_value == "OFF"){secondary_value = "ON";}
else{secondary_value = "OFF";}
    String wrfile4 = "/secondary_value";
    File fw4 = SPIFFS.open(wrfile4.c_str(), "w");
    fw4.println( secondary_value );
    fw4.close();
    request->send(SPIFFS, "/setup.html", String(), false, processor); });

  // MAPセットの際のPOST処理
  server.on("/set_map", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    map_value = request->arg("map_param");
    code_value = map_value.toInt();
    String writeStr2 = map_value;
    String wrfile2 = "/code_value";
    File fw2 = SPIFFS.open(wrfile2.c_str(), "w");
    fw2.println( writeStr2 );
    fw2.close();
    if (code_value == 1) {
      digitalWrite(CODE_PIN1, LOW );
      digitalWrite(CODE_PIN2, LOW );
    }
    if (code_value == 2) {
      digitalWrite(CODE_PIN1, HIGH );
      digitalWrite(CODE_PIN2, LOW );
    }
    if (code_value == 3) {
      digitalWrite(CODE_PIN1, LOW );
      digitalWrite(CODE_PIN2, HIGH );
    } 
    request->send(SPIFFS, "/setup.html", String(), false, processor); });

  ///////// Graph
  //  Graph Interval
  server.on("/graph_sw", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    graph_sw = request->arg("graph_param");
    if (graph_sw == "30s"){
    request->send(SPIFFS, "/graph30s.html");}
        if (graph_sw == "2m"){
    request->send(SPIFFS, "/graph2m.html");}
        if (graph_sw == "/"){
    request->send(SPIFFS, "/setup.html",String(), false, processor);} });

  server.on("/chart.js.gz", HTTP_GET, [](AsyncWebServerRequest *request)
            { AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/chart.js.gz", "text/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response); });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/style.css", "text/css");
 //   response->addHeader("Content-Encoding", "gzip");
    request->send(response); });

  server.on("/o2_in1", HTTP_GET, [](AsyncWebServerRequest *request)
            {  String o2_in1_str = String(in_volt1 * 1000 ) ;
        request->send_P(200, "text/plain", o2_in1_str.c_str()); });

  server.on("/o2_out1", HTTP_GET, [](AsyncWebServerRequest *request)
            { String o2_out1_str = String(serial_out_volt1 * 1000) ;
        request->send_P(200, "text/plain", o2_out1_str.c_str()); });

  server.on(
      "/o2_in2", HTTP_GET, [](AsyncWebServerRequest *request)
      { String secondary_sw = secondary_value;
        if(secondary_value == "ON"){
              String o2_in2_str = String(in_volt2 * 1000 ) ;
             request->send_P(200, "text/plain", o2_in2_str.c_str());} });

  server.on(
      "/o2_out2", HTTP_GET, [](AsyncWebServerRequest *request)
      { if(secondary_value == "ON"){
              String o2_out2_str = String(serial_out_volt2 * 1000) ;
              request->send_P(200, "text/plain", o2_out2_str.c_str()); }});

  // サーバースタート
  //  Serial.begin(115200);
  AsyncElegantOTA.begin(&server);
  WebSerial.begin(&server);
  server.begin();
}

void loop()
{
  int o2_value1 = analogRead(O2IN_PIN1);
  int o2_value2 = analogRead(O2IN_PIN2);
  in_volt1 = (o2_value1 + 201) / 1230.0;
  in_volt2 = (o2_value2 + 201) / 1230.0;
  serial_out_volt1 = in_volt1 + shift_value;
  serial_out_volt2 = in_volt2 + shift_value;
  int out_duty1 = (in_volt1 + shift_value) * 2530 / 30 - 8;
  int out_duty2 = (in_volt1 + shift_value) * 2530 / 30 - 8;

  if (af_value == "-0.01")
  {
    out_duty1 = 35;
    out_duty2 = 35;
    serial_out_volt1 = 0.45;
    serial_out_volt1 = 0.45;
  }
  else
  {
    if (serial_out_volt1 < 0.1)
    {
      serial_out_volt1 = 0.1;
    }
    if (serial_out_volt1 > 0.9)
    {
      serial_out_volt1 = 0.9;
    }
    if (serial_out_volt2 < 0.1)
    {
      serial_out_volt2 = 0.1;
    }
    if (serial_out_volt2 > 0.9)
    {
      serial_out_volt2 = 0.9;
    }

    if (out_duty1 < 8)
    {
      out_duty1 = 8;
    }
    if (out_duty1 > 70)
    {
      out_duty1 = 70;
    }

    if (out_duty2 < 8)
    {
      out_duty2 = 8;
    }
    if (out_duty2 > 70)
    {
      out_duty2 = 70;
    }
  }
  dacWrite(O2OUT_PIN1, out_duty1);
  dacWrite(O2OUT_PIN2, out_duty2);

  // Write log
  if (counter == 1 && log_sw == "ON")
  {
    File fileToAppend = SPIFFS.open("/ls-oat_log.csv", FILE_APPEND);
    fileToAppend.print(millis());
    fileToAppend.print(",");
    fileToAppend.print(in_volt1);
    fileToAppend.print(",");
    fileToAppend.print(serial_out_volt1);
    fileToAppend.print(",");
    fileToAppend.print(in_volt2);
    fileToAppend.print(",");
    fileToAppend.print(serial_out_volt2);
    fileToAppend.print(",");
    fileToAppend.print(shift_value);
    fileToAppend.print(",");
    fileToAppend.println(code_value);
    fileToAppend.close();
    counter = 0;
  }
  else
  {
    counter = 1;
  }

  // Write Web Serial
  String web_str = String(o2_value1);
  web_str += ",";
  web_str += String(o2_value1);
  web_str += ",";
  web_str += String(out_duty1);
  web_str += ",";
  web_str += String(out_duty2);
  web_str += ",";
  web_str += (secondary_value);
  //  web_str += ",";
 // web_str += (secondary_sw);
  WebSerial.println(web_str);

  // Duration of update in_volt1age value
  delay(update_frequency);
}
