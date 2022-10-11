///////// Description //////////
/*

*/
#include <Arduino.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <WebSerial.h>
#include <AsyncElegantOTA.h>
#include <ESP32Ping.h>
#define FORMAT_SPIFFS_IF_FAILED true

///// Firm version
const String firm_version = "2.0.8";

///// WiFi Config /////
const char ap_ssid[] = "LS20-AP0000";
const char ap_pass[] = "NJdcTw5gxU"; //
const IPAddress ap_ip(10, 1, 1, 1);
const IPAddress ap_subnet(255, 255, 255, 0);
const IPAddress dns(8, 8, 8, 8);
AsyncWebServer server(80); // 

String ssid;
String wifi_pass;
int ip_data0;
int ip_data1;
int ip_data2;
int ip_data3;
int subnet_data0;
int subnet_data1;
int subnet_data2;
int subnet_data3;
int gateway_data0;
int gateway_data1;
int gateway_data2;
int gateway_data3;

// Sensor input cycle
const int update_frequency = 50;

// These are HW configuration.  Don't change this value
const int O2IN_PIN1 = 32;
const int O2IN_PIN2 = 35;
const int O2OUT_PIN1 = 25;
const int O2OUT_PIN2 = 26;
const int CODE_PIN1 = 18;
const int CODE_PIN2 = 19;

// Variable for rewrite web pages. 
String file_version;
String af_value;
String map_value;
String secondary_value = "OFF";
String log_sw = "OFF";
String graph_sw;
String wifi_mode;
String graph_duration;
String ping_status;

int shift_value;
int serial_out_volt1;
int serial_out_volt2;
int in_volt1;
int in_volt2;
int code_value;
int counter;
int reset_code = 0;

///// Function for rewrite web pages 
String processor(const String &var)
{
  if (var == "AF_STATE")
  {
    if (af_value == "0.01"){
      return "Disable";
    }
    else{
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

  if (var == "WIFI_MODE")
  {
    return wifi_mode;
  }

  if (var == "FIRM_VIRSION")
  {
    return firm_version;
  }
  if (var == "FILE_VERSION")
  {
    return file_version;
  }

  
  if (var == "RELOAD_BTN")
  {
    return graph_sw;
  }

  if (var == "SECONDARY_OFF1" && secondary_value == "OFF")
  {
    return "<!--";
  }
  if (var == "SECONDARY_OFF2" && secondary_value == "OFF")
  {
    return "-->";
  }
  if (var == "GRAPH_DURATION")
  {
    return graph_duration;
  }

  if (var == "GRAPH_BUTTON_COLOR1")
  {
    if (graph_sw == "30s")
    {
      return "gray";
    }
    else
    {
      return "orange";
    }
  }

  if (var == "GRAPH_BUTTON_COLOR2")
  {
    if (graph_sw == "2m")
    {
      return "gray";
    }
    else
    {
      return "orange";
    }
  }
  if (var == "GRAPH_BUTTON_COLOR3")
  {
    if (secondary_value == "ON")
    {
      return "▼";
    }
    else
    {
      return "▶";
    }
  }
  if (var == "SECONDARY_PARAM")
  {
    return secondary_value;
  }

  //
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

  /////
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

  // For WiFi configration page.
  if (var == "SSID")
  {
    return ssid;
  }
  if (var == "PASSWD")
  {
    return wifi_pass;
  }
  if (var == "IP_DATA0")
  {
    return String(ip_data0);
  }

  if (var == "IP_DATA1")
  {
    return String(ip_data1);
  }
  if (var == "IP_DATA2")
  {
    return String(ip_data2);
  }
  if (var == "IP_DATA3")
  {
    return String(ip_data3);
  }
  if (var == "SUBNET_DATA0")
  {
    return String(subnet_data0);
  }

  if (var == "SUBNET_DATA1")
  {
    return String(subnet_data1);
  }
  if (var == "SUBNET_DATA2")
  {
    return String(subnet_data2);
  }
  if (var == "SUBNET_DATA3")
  {
    return String(subnet_data3);
  }

  if (var == "GATEWAY_DATA0")
  {
    return String(gateway_data0);
  }

  if (var == "GATEWAY_DATA1")
  {
    return String(gateway_data1);
  }
  if (var == "GATEWAY_DATA2")
  {
    return String(gateway_data2);
  }
  if (var == "GATEWAY_DATA3")
  {
    return String(gateway_data3);
  }
  if (var == "WIFI_STATUS")
  {
    return ping_status;
  }

  return String();
}

void write_setup_config()
{
  String wrfile1 = "/setup_config";
  File fw1 = SPIFFS.open(wrfile1.c_str(), "w");
  fw1.println(af_value);
  fw1.println(map_value);
  fw1.println(file_version);
  fw1.close();
}

void write_wifi_config()
{
  String wrfile2 = "/wifi_config";
  File fw2 = SPIFFS.open(wrfile2.c_str(), "w");
  fw2.println(ssid);
  fw2.println(wifi_pass);
  fw2.print(ip_data0);
  fw2.print(".");
  fw2.print(ip_data1);
  fw2.print(".");
  fw2.print(ip_data2);
  fw2.print(".");
  fw2.println(ip_data3);
  fw2.print(subnet_data0);
  fw2.print(".");
  fw2.print(subnet_data1);
  fw2.print(".");
  fw2.print(subnet_data2);
  fw2.print(".");
  fw2.println(subnet_data3);
  fw2.print(gateway_data0);
  fw2.print(".");
  fw2.print(gateway_data1);
  fw2.print(".");
  fw2.print(gateway_data2);
  fw2.print(".");
  fw2.println(gateway_data3);
  fw2.close();

  /*  Serial.println(ssid);
    Serial.println(wifi_pass);
    Serial.print(ip_data0);
    Serial.print(".");
    Serial.print(ip_data1);
    Serial.print(".");
    Serial.print(ip_data2);
    Serial.print(".");
    Serial.println(ip_data3);
    Serial.print(subnet_data0);
    Serial.print(".");
    Serial.print(subnet_data1);
    Serial.print(".");
    Serial.print(subnet_data2);
    Serial.print(".");
    Serial.println(subnet_data3);
    Serial.print(gateway_data0);
    Serial.print(".");
    Serial.print(gateway_data1);
    Serial.print(".");
    Serial.print(gateway_data2);
    Serial.print(".");
    Serial.println(gateway_data3);*/
}

int split(String data, char delimiter, String *dst)
{
  int index = 0;
  int arraySize = (sizeof(data) / sizeof((data)[0]));
  int datalength = data.length();
  for (int i = 0; i < datalength; i++)
  {
    char tmp = data.charAt(i);
    if (tmp == delimiter)
    {
      index++;
      if (index > (arraySize - 1))
        return -1;
    }
    else
      dst[index] += tmp;
  }
  return (index + 1);
}
void start_ap()
{
  Serial.println("wifi connection timeout, AP Start");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.softAP(ap_ssid, ap_pass);              // SSIDとパスの設定
  delay(100);                                 // このdelayを入れないと失敗する場合がある
  WiFi.softAPConfig(ap_ip, ap_ip, ap_subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定
  IPAddress myIP = WiFi.softAPIP();           // WiFi.softAPIP()でWiFi起動
      Serial.println("AP Connected");
    Serial.println(myIP);
}

///////////////////////////// Execution Part //////////////////////////////
void setup()
{
  Serial.begin(115200);
  Serial.println("");

  pinMode(O2OUT_PIN1, OUTPUT);
  pinMode(O2OUT_PIN2, OUTPUT);
  pinMode(CODE_PIN1, OUTPUT);
  pinMode(CODE_PIN2, OUTPUT);
  analogSetAttenuation(ADC_6db);  
  analogSetWidth(10);


  // SPIFFSのセットアップ
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  /////ログファイル準備
  SPIFFS.remove("/ls-oat_log.csv");
  File fileToWrite = SPIFFS.open("/ls-oat_log.csv", FILE_WRITE);
  fileToWrite.println("time(mil),in_volt1,out_volt1,in_volt2,out_volt2,shift_voltage(mV),map");
  fileToWrite.close();

  ///// Setup Config読み取り
  String wrfile1 = "/setup_config";
  File fr1 = SPIFFS.open(wrfile1.c_str(), "r");
  af_value = fr1.readStringUntil('\n');
  af_value.trim();
  shift_value = af_value.toFloat() * 1000;
  map_value = fr1.readStringUntil('\n');
  map_value.trim();
  code_value = map_value.toInt();
  file_version = fr1.readStringUntil('\n');
  fr1.close();

  //////
  String wrfile2 = "/wifi_config";
  File fr2 = SPIFFS.open(wrfile2.c_str(), "r");
  ssid = fr2.readStringUntil('\n');
  ssid.trim();
  wifi_pass = fr2.readStringUntil('\n');
  wifi_pass.trim();
  String str3 = fr2.readStringUntil('\n');
  String ip_data[4];
  split(str3, '.', ip_data);
  ip_data0 = ip_data[0].toInt();
  ip_data1 = ip_data[1].toInt();
  ip_data2 = ip_data[2].toInt();
  ip_data3 = ip_data[3].toInt();
  const IPAddress ip(ip_data0, ip_data1, ip_data2, ip_data3);

  String str4 = fr2.readStringUntil('\n');
  String subnet_data[4];
  split(str4, '.', subnet_data);
  subnet_data0 = subnet_data[0].toInt();
  subnet_data1 = subnet_data[1].toInt();
  subnet_data2 = subnet_data[2].toInt();
  subnet_data3 = subnet_data[3].toInt();
  const IPAddress subnet(subnet_data0, subnet_data1, subnet_data2, subnet_data3);

  String str5 = fr2.readStringUntil('\n');
  String gateway_data[4];
  split(str5, '.', gateway_data);
  gateway_data0 = gateway_data[0].toInt();
  gateway_data1 = gateway_data[1].toInt();
  gateway_data2 = gateway_data[2].toInt();
  gateway_data3 = gateway_data[3].toInt();
  const IPAddress gateway(gateway_data0, gateway_data1, gateway_data2, gateway_data3);

  fr2.close();

  Serial.println(ssid);
  Serial.println(String(wifi_pass));

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

  // Wifiのセットアップ
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), wifi_pass.c_str()); //  接続確立まで待つこと
  Serial.println("Connecting...");
  int wifi_count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting...");
    wifi_count++;
    if (wifi_count > 10)
    {
      start_ap();
      ping_status = "WiFi:<font color=\"red\"><B>NG</B></font>";
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.config(ip, gateway, subnet, dns);
    Serial.println("Connected");
    Serial.println(WiFi.localIP());
  }
  bool ping_gateway = Ping.ping(gateway, 2);
  bool ping_dns = Ping.ping(dns, 2);
  if (!ping_gateway)
  {
    if (ping_status == "")
    {
      ping_status = "WiFi:<font color=\"red\"><B>OK</B></font> / Gateway:<B>NG</B></font>";
    }
    start_ap();
    wifi_mode = "AP";
  }
  else
  {
    if (!ping_dns)
    {
      ping_status = "WiFI:<font color=\"red\"><B>OK</B></font> / Gateway:<font color=\"red\"><B>OK</B></font> / Internet:<font color=\"red\"><B>NG</B></font>";
      wifi_mode = "Client";
    }
    else
    {
      ping_status = "WiFi:<font color=\"red\"><B>OK</B></font> / Gateway:<font color=\"red\"><B>OK</B></font> / Internet:<font color=\"red\"><B>OK</B></font>";
      wifi_mode = "Client";
    }
  }



  // GETリクエストに対するハンドラーを登録 rootにアクセスされた時のレスポンス


  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/setup.html", String(), false, processor); });

  server.on("/ls_logo", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/ls_logo.png", "image/png"); });

  server.on("/uploader.png", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/uploader.png", "image/png"); });

  server.on("/favicon", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/favicon.ico", "image/x-icon"); });

  // WIFI設定のダウンロード
  server.on("/wifi_config", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/wifi_config", "text/csv"); });

  // デバイス設定のダウンロード
  server.on("/setup_config", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/setup_config", "text/csv"); });

  //  Log on or  off
  server.on("/log_sw", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
             if(log_sw == "OFF"){log_sw = "ON";}
              else{log_sw = "OFF";}    
            request->send(SPIFFS, "/setup.html", String(), false, processor); });

  server.on("/ls-oat_log.csv", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/ls-oat_log.csv", "text/csv"); });

  // AF setting
  server.on("/set_af", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
    af_value = request->arg("af_param");
    if(af_value != ""){
   shift_value = af_value.toFloat() * 1000;
    write_setup_config();}
    request->send(SPIFFS, "/setup.html", String(), false, processor); });

  // MAPセットの際のPOST処理
  server.on("/set_map", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
    map_value = request->arg("map_param");
    if(map_value != ""){
    code_value = map_value.toInt();
      write_setup_config();}
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
  server.on("/graph_sw", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
    graph_sw = request->arg("graph_param");
    if (graph_sw == "30s"){ graph_duration = 160;
    request->send(SPIFFS, "/graph.html",String(),false,processor);}
        if (graph_sw == "2m"){ graph_duration = 470;
    request->send(SPIFFS, "/graph.html",String(),false,processor);}
        if (graph_sw == "/" || graph_sw == ""){
    request->send(SPIFFS, "/setup.html",String(), false, processor);} 
            });

  server.on("/secondary_sw", HTTP_ANY, [](AsyncWebServerRequest *request)
            {
    if(secondary_value == "OFF"){secondary_value = "ON";}
    else{secondary_value = "OFF";}
    request->send(SPIFFS, "/graph.html",String(),false,processor);
      });


  server.on("/chart.js.gz", HTTP_GET, [](AsyncWebServerRequest *request)
            { AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/chart.js.gz", "text/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response); });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/style.css", "text/css");
 //   response->addHeader("Content-Encoding", "gzip");
    request->send(response); });

  server.on("/o2_in1", HTTP_GET, [](AsyncWebServerRequest *request)
            {  String o2_in1_str = String(in_volt1 ) ;
        request->send_P(200, "text/plain", o2_in1_str.c_str()); });

  server.on("/o2_out1", HTTP_GET, [](AsyncWebServerRequest *request)
            { String o2_out1_str = String(serial_out_volt1) ;
        request->send_P(200, "text/plain", o2_out1_str.c_str()); });

  server.on(
      "/o2_in2", HTTP_GET, [](AsyncWebServerRequest *request)
      { if(secondary_value == "ON"){
              String o2_in2_str = String(in_volt2 ) ;
             request->send_P(200, "text/plain", o2_in2_str.c_str());} });

  server.on(
      "/o2_out2", HTTP_GET, [](AsyncWebServerRequest *request)
      { if(secondary_value == "ON"){
              String o2_out2_str = String(serial_out_volt2) ;
              request->send_P(200, "text/plain", o2_out2_str.c_str()); } });

  server.on("/set-wifi", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/set-wifi.html", String(), false, processor); });

  server.on("/set-wifi", HTTP_POST, [](AsyncWebServerRequest *request)
            {   ssid = request->arg("ssid");
                  wifi_pass = request->arg("wifi_pass");
                  String ip_data0_str = request->arg("ip_data0");
                  ip_data0 = ip_data0_str.toInt();
                  String ip_data1_str = request->arg("ip_data1");
                  ip_data1 = ip_data1_str.toInt();                  
                  String ip_data2_str = request->arg("ip_data2");
                  ip_data2 = ip_data2_str.toInt();
                  String ip_data3_str = request->arg("ip_data3");
                  ip_data3 = ip_data3_str.toInt();        

                  String subnet_data0_str = request->arg("subnet_data0");
                  subnet_data0 = subnet_data0_str.toInt();
                  String subnet_data1_str = request->arg("subnet_data1");
                  subnet_data1 = subnet_data1_str.toInt();
                  String subnet_data2_str = request->arg("subnet_data2");
                  subnet_data2 = subnet_data2_str.toInt();
                  String subnet_data3_str = request->arg("subnet_data3");
                  subnet_data3 = subnet_data3_str.toInt();

                  String gateway_data0_str = request->arg("gateway_data0");
                  gateway_data0 = gateway_data0_str.toInt();
                  String gateway_data1_str = request->arg("gateway_data1");
                  gateway_data1 = gateway_data1_str.toInt();
                  String gateway_data2_str = request->arg("gateway_data2");
                  gateway_data2 = gateway_data2_str.toInt();
                  String gateway_data3_str = request->arg("gateway_data3");
                  gateway_data3 = gateway_data3_str.toInt();
              write_wifi_config();
              reset_code = 1;
              request->send(SPIFFS, "/setup.html", String(), false, processor); });

  // サーバースタート
  AsyncElegantOTA.begin(&server);
  WebSerial.begin(&server);
  server.begin();
}

void loop()
{
  int o2_value1 = analogRead(O2IN_PIN1);
  int o2_value2 = analogRead(O2IN_PIN2);
  // in_volt1 = (o2_value1 + 200) / 4092.0 * 2.0;
 in_volt1 =  (in_volt1 * 3 + (o2_value1 + 270) * 1825 / 4096) / 4  ;
 in_volt2 =  (in_volt2 * 3 + (o2_value2 + 270) * 1825 / 4096) / 4  ;

serial_out_volt1 = in_volt1 + shift_value;
serial_out_volt2 = in_volt2 + shift_value;

  int out_duty1 = (in_volt1 + shift_value) * 256 / 3070 - 6;
  int out_duty2 = (in_volt1 + shift_value) * 256 / 3070 - 6;

  if (af_value == "0.01")
  {
    out_duty1 = 35;
    out_duty2 = 35;
    serial_out_volt1 = 450;
    serial_out_volt1 = 450;
  }
  else
  {
    if (serial_out_volt1 < 100)
    {
      serial_out_volt1 = 100;
      out_duty1 = 1;
    }
    if (serial_out_volt1 > 900)
    {
      serial_out_volt1 = 900;
      out_duty1 = 67;
    }
    if (serial_out_volt2 < 100)
    {
      serial_out_volt2 = 100;
       out_duty2 = 1;
    }
    if (serial_out_volt2 > 900)
    {
      serial_out_volt2 = 900;
      out_duty2 = 67;
    }
  }
  dacWrite(O2OUT_PIN1, out_duty1);
  dacWrite(O2OUT_PIN2, out_duty2);

  // Write log
  if (counter >= 10 && log_sw == "ON")
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
    counter = 0;
  }
  else
  {
    counter = ++counter;
  }

  // Duration of update in_volt1age value
  delay(update_frequency);

  if (reset_code == 1)
  {
    delay(1000);
    ESP.restart();
  }
}
