///////// Description //////////
/*
  This is a lambda shift device for engines with narrow band sensors.
  Normally, the ECU judges the O2 sensor output of 0.4 to 0.5v as A/F 14.7,
  below 0.4v as lean, and above 0.5v as rich.

  This device receives the output from the O2 sensor at A6, shifts the in_voltage to be sent to the ECU, and outputs it to D3.
  Three different values are shifted by switches.
  -SW1 On: A/F ratio 14.0 Best power
  -SW2 On: A/F ratio 15.4 Best economy
  SW Off: A/F Ratio: 14.7 Stock The amount to be shifted can be adjusted by changing variables.

  STATUS_LED
  For detecting O2 sensor failure and checking switch settings.
  When the O2 sensor input is 0.1v or less (when the engine is stopped), the following operation is performed.
  -SW1 on: Flashes every 0.2 seconds
  -SW2 on: Flashes every 1 second
  -SW off: Flashes every 0.5 sec.
  If the blinking continues even after the engine is turned on, there is a possibility of 02 sensor failure.
  If the blinking interval does not change even after switching the switch, the switch may be faulty.

  Fuel_LED
  Lights up when the rich signal is sent to the ECU. You can check if it is working properly as a system.
  If the light does not come on when the engine is started and the accelerator pedal is opened, there may be some kind of error.
*/

///// A/F Config /////
// If you want to change A/F, Change value.
//-0.30 15.4
//-0.20 15.1
//-0.10 14.9
// 0.00 14.7
// 0.10 14.5
// 0.20 14.3
// 0.30 14.0
// 0.40 13.6-

const float sw1_shift = 0.30;
const float sw2_shift = 0.40;
const float swoff_shift = 0;

////////// Program ///////////
///// Update duration ////
const int update_frequency = 100;

//These are HW configuration.  Don't change this value
//
const int O2IN_PIN1 = 27;
const int O2IN_PIN2 = 13;
const int O2OUT_PIN1 = 25;
const int SW_PIN1 = 18;
const int SW_PIN2 = 19;
const int STATUS_PIN1 = 32;
const int FUEL_PIN1 = 22;
const int CODE_PIN1 = 23;
const int CODE_PIN2 = 33;

const int out_duty_low = 3;
const int out_duty_mid = 29;
const int out_duty_high = 63;
const float  min_volt = 0.14;

////////// Execution Part //////////
void setup() {
  Serial.begin( 9600 );
  pinMode( STATUS_PIN1, OUTPUT );
  //  pinMode( FUEL_PIN1, OUTPUT );
  pinMode( O2OUT_PIN1, OUTPUT );
  pinMode( SW_PIN1, INPUT_PULLUP );
  pinMode( SW_PIN2, INPUT_PULLUP );
  pinMode( CODE_PIN1, OUTPUT );
  pinMode( CODE_PIN2, OUTPUT );

  ledcSetup(0, 12800, 8);
  ledcAttachPin(FUEL_PIN1, 0);
}

void loop() {
  //  digitalWrite(FUEL_PIN1, HIGH);
  int o2_value1 = analogRead( O2IN_PIN1 );
  int o2_value2 = analogRead( O2IN_PIN2 );
  int sw1_value = digitalRead( SW_PIN1 );
  int sw2_value = digitalRead( SW_PIN2 );
  //  float in_volt = o2_value1 * 3.9 / 4096.0;
  //  float in_volt = (o2_value1 + 161) / 1260.0;
  float in_volt = (o2_value1 + 161) / 1230.0;

  float out_volt;
  float serial_out_volt;

  // Status LED turning on
  digitalWrite(STATUS_PIN1, HIGH);

  ///// SW1
  // Cording Plug
  digitalWrite(CODE_PIN1, HIGH );
  digitalWrite(CODE_PIN2, LOW );

  if (sw1_value == 0 ) {
    // OUTPUT to ECU and Rich LED
    serial_out_volt = out_proc(in_volt, sw1_shift);

    // STATUS LED　
    if (in_volt < min_volt || in_volt > 1.2) {
      delay(200);
      digitalWrite(STATUS_PIN1, LOW );
    }
  }

  ///// SW OFF
  if (sw1_value == 1 && sw2_value == 1 ) {
    // Cording Plug
    digitalWrite(CODE_PIN1, LOW );
    digitalWrite(CODE_PIN2, LOW );

    // OUTPUT to ECU and Rich LED
    serial_out_volt = out_proc(in_volt, swoff_shift);

    ///// STATUS LED　
    if (in_volt < min_volt || in_volt > 1.2) {
      delay(500);
      digitalWrite(STATUS_PIN1, LOW);
    }
  }

  // SW2
  if (sw2_value == 0 ) {
    // Cording Plug
    digitalWrite(CODE_PIN1, LOW );
    digitalWrite(CODE_PIN2, HIGH );

    // OUTPUT to ECU and Rich LED
    serial_out_volt = out_proc(in_volt, sw2_shift);

    // STATUS LED　
    if (in_volt < min_volt || in_volt > 1.2) {
      delay(1000);
      digitalWrite(STATUS_PIN1, LOW);
    }
  }
  // for logging //
  Serial.print( o2_value1 );
  Serial.print( "," );
  Serial.print( o2_value2 );
  Serial.print( "," );
  Serial.print( sw1_value );
  Serial.print( "," );
  Serial.print( sw2_value );
  Serial.print( "," );
  Serial.print( in_volt );
  Serial.print( "," );
  Serial.println ( serial_out_volt );

  // Duration of update in_voltage value
  delay(update_frequency);
}

//The voltage from the 02 sensor is input, the shift amount is added to the set air-fuel ratio,
//the output voltage to the ECU, and the FUEL LED is turned on or off.
float out_proc (float in_volt, float shift_value) {
  float out_volt;

  if (in_volt <= 0.43 + shift_value) {
    ledcWrite(0, 0 );
    dacWrite( O2OUT_PIN1, out_duty_low);
    out_volt = (out_duty_low + 9.0) / 83.0;
  }
  if (in_volt >= 0.43 + shift_value && in_volt < 0.47 + shift_value) {
    ledcWrite(0, 40 );
    out_volt = (out_duty_mid + 9.0) / 83.0;
  }

  if (in_volt >= 0.47 + shift_value) {
    ledcWrite(0, 255 );
    out_volt = (out_duty_high + 9.0) / 83.0;
  }
  return (out_volt);
}
