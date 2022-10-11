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

#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED true
// #include "FS.h"

///// WiFi Config /////
const char ssid[] = "LS-OTA-AP";
const char pass[] = "12345678";       // パスワードは8文字以上
const IPAddress ip(10, 1, 1, 1);
const IPAddress subnet(255, 255, 255, 0);
AsyncWebServer server(80);            // ポート設定


///// Update duration ////
const int update_frequency = 100;

//These are HW configuration.  Don't change this value
const int O2IN_PIN1 = 32;
const int O2IN_PIN2 = 35;
const int O2OUT_PIN1 = 25;
const int O2OUT_PIN2 = 26;
const int STATUS_PIN1 = 22;
const int FUEL_PIN1 = 27;
const int CODE_PIN1 = 23;
const int CODE_PIN2 = 13;

const int out_duty_low = 3;
const int out_duty_mid = 29;
const int out_duty_high = 63;
const float  min_volt = 0.17;

String af_value;
String map_value;
float shift_value;
int code_value;

////////// Execution Part //////////
void setup() {
  Serial.begin(9600);
  ledcSetup(0, 12800, 8);
  ledcAttachPin(FUEL_PIN1, 0);
  pinMode( STATUS_PIN1, OUTPUT );
  pinMode( O2OUT_PIN1, OUTPUT );
  pinMode( O2OUT_PIN2, OUTPUT );
  pinMode( CODE_PIN1, OUTPUT );
  pinMode( CODE_PIN2, OUTPUT );

  WiFi.softAP(ssid, pass);           // SSIDとパスの設定
  delay(100);                        // このdelayを入れないと失敗する場合がある
  WiFi.softAPConfig(ip, ip, subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定
  IPAddress myIP = WiFi.softAPIP();  // WiFi.softAPIP()でWiFi起動

  // 各種情報を表示
  /*  Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("AP IP address: ");
    Serial.println(myIP); */

  // SPIFFSのセットアップ
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  ////保存したシフト値読み取り
  String wrfile1 = "/shift_value";
  File fr1 = SPIFFS.open(wrfile1.c_str(), "r");
  String readStr1 = fr1.readStringUntil('\n');
  af_value = readStr1;
  shift_value = readStr1.toFloat();
  fr1.close();

  ////保存したコード値読み取り
  String wrfile2 = "/code_value";
  File fr2 = SPIFFS.open(wrfile2.c_str(), "r");
  String readStr2 = fr2.readStringUntil('\n');
  map_value = readStr2;
  code_value = readStr2.toInt();
  fr2.close();
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


  // GETリクエストに対するハンドラーを登録 rootにアクセスされた時のレスポンス
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/setup.html", String(), false, processor);
  });

  // style.cssのレスポンス
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  //AFセットの際のPOST処理
  server.on("/set_af", HTTP_POST, [](AsyncWebServerRequest * request) {
    af_value = request->arg("af_param");
    shift_value = af_value.toFloat();
    String wrfile1 = "/shift_value";
    String writeStr1 = af_value;
    File fw1 = SPIFFS.open(wrfile1.c_str(), "w");
    fw1.println( writeStr1 );
    fw1.close();
    request->send(SPIFFS, "/setup.html", String(), false, processor);
  });

  //MAPセットの際のPOST処理
  server.on("/set_map", HTTP_POST, [](AsyncWebServerRequest * request) {
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
    request->send(SPIFFS, "/setup.html", String(), false, processor);
  });

  // サーバースタート
  server.begin();
}

void loop() {
  int o2_value1 = analogRead( O2IN_PIN1 );
  int o2_value2 = analogRead( O2IN_PIN2 );
  float in_volt1 = (o2_value1 + 201) / 1230.0;
  float in_volt2 = (o2_value2 + 201) / 1230.0;

  float out_volt;
  float serial_out_volt1 = out_proc(in_volt1, shift_value, O2OUT_PIN1);
  float serial_out_volt2 = out_proc(in_volt2, shift_value, O2OUT_PIN2);

  // Status LED turning on
  digitalWrite(STATUS_PIN1, HIGH);

  // STATUS LED　O2sensor error indicate
  if (in_volt1 < min_volt || in_volt1 > 1.2 || in_volt2 < min_volt || in_volt2 > 1.2) {
    delay(200);
    digitalWrite(STATUS_PIN1, LOW );
  }

  // for Serial logging //
  Serial.print( in_volt1 );
  Serial.print( "," );
  Serial.print ( serial_out_volt1 );
  Serial.print( "," );
  Serial.print( in_volt2 );
  Serial.print( "," );
  Serial.print ( serial_out_volt2 );
  Serial.print (",");
  Serial.print(shift_value);
  Serial.print( "," );
  Serial.println( code_value );
  /*  Serial.print( "," );
    Serial.print(af_value);
    Serial.print( "," );
    Serial.println( map_value ); */

  // Duration of update in_volt1age value
  delay(update_frequency);
}

//The voltage from the 02 sensor is input, the shift amount is added to the set air-fuel ratio,
//the output voltage to the ECU, and the FUEL LED is turned on or off.
float out_proc (float in_volt, float shift_value, int out_pin) {
  float out_volt;
  float out_duty;

  if (in_volt <= 0.43 + shift_value) {
    ledcWrite(0, 0 );
    out_duty = out_duty_low;
  }
  if (in_volt >= 0.43 + shift_value && in_volt < 0.47 + shift_value) {
    ledcWrite(0, 40 );
    out_duty = out_duty_mid;
  }
  if (in_volt >= 0.47 + shift_value) {
    ledcWrite(0, 255 );
    out_duty = out_duty_high;
  }
  dacWrite( out_pin, out_duty);
  out_volt = (out_duty + 8.0) / 84.0;
  return (out_volt);
}

String processor(const String& var) {
  //  Serial.println(var);
  if (var == "AF_STATE") {
    return af_value;
  }
  if (var == "MAP_STATE") {
    return map_value;
  }
  return String();
}
