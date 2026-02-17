#include <IRremote.h>
#include <math.h>

// === Pin Definitions ===
#define IR_RECEIVE_PIN 11
#define MOTOR_PWM 5
#define MOTOR_IN1 9
#define MOTOR_IN2 10
#define TEMP_PIN A0

#define SHIFT_DATA 2
#define SHIFT_CLOCK 3
#define SHIFT_LATCH 4

// === Remote Buttons ===
#define AUTO_MODE_BUTTON    0xF30CFF00 // OK
#define FAN_OFF_BUTTON      0xB54AFF00   // 0
#define SPEED_UP_BUTTON     0xEA15FF00 // +
#define SPEED_DOWN_BUTTON   0xF807FF00 // -
#define DIR_FORWARD_BUTTON  0xBF40FF00 // >>
#define DIR_REVERSE_BUTTON  0xBB44FF00 // <<
#define START_BUTTON        0xE718FF00 // Play

// === Constants ===
const float FAN_ON_TEMP = 24.5;
const float FAN_OFF_TEMP = 23.5;
const float MAX_TEMP = 35.0;
const float MIN_TEMP = FAN_ON_TEMP;
const int PWM_MIN = 100;
const int PWM_MAX = 255;

bool autoMode = false;
bool fanOn = false;
bool overrideStart = false;
int manualPWM = 0;
bool forward = true;

unsigned long lastTempPrint = 0;

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  pinMode(MOTOR_PWM, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  setDirection(true);
  analogWrite(MOTOR_PWM, 0);
  updateBarGraph(0); // All LEDs off
}

void loop() {
  // === Print Temperature Every 1 Second ===
  if (millis() - lastTempPrint >= 1000) {
    float tempC = readTemperatureC();
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" °C");
    lastTempPrint = millis();
  }

  // === Handle IR Remote ===
  if (IrReceiver.decode()) {
    unsigned long raw = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("IR Code: 0x"); Serial.println(raw, HEX);

    if (raw == START_BUTTON) {
      overrideStart = true;
      autoMode = false;
      manualPWM = 180;
      Serial.println("Mode: START - Fan ON at medium speed");
    } else {
      overrideStart = false;

      if (raw == AUTO_MODE_BUTTON) {
        autoMode = !autoMode;
        Serial.println(autoMode ? "AUTO MODE: TURNED ON" : "AUTO MODE: TURNED OFF");
      } else if (raw == FAN_OFF_BUTTON) {
        manualPWM = 0;
        Serial.println("Command: FAN OFF");
      } else if (raw == SPEED_UP_BUTTON) {
        manualPWM = min(manualPWM + 25, 255);
        Serial.print("Command: SPEED INCREASED - PWM = ");
        Serial.println(manualPWM);
      } else if (raw == SPEED_DOWN_BUTTON) {
        manualPWM = max(manualPWM - 25, 0);
        Serial.print("Command: SPEED DECREASED - PWM = ");
        Serial.println(manualPWM);
      } else if (raw == DIR_FORWARD_BUTTON) {
        setDirection(true);
        Serial.println("Direction: FORWARD");
      } else if (raw == DIR_REVERSE_BUTTON) {
        setDirection(false);
        Serial.println("Direction: REVERSE");
      } else {
        Serial.println("Unknown Button");
      }
    }

    IrReceiver.resume();  // Ready for next signal
  }

  // === Control Fan Speed Based on Mode ===
  int pwmToSet = 0;
  bool fanRunning = false;
  float tempC = readTemperatureC();

  if (overrideStart) {
    pwmToSet = 180;
    fanRunning = true;
  } else if (autoMode) {
    if (!fanOn && tempC >= FAN_ON_TEMP) fanOn = true;
    if (fanOn && tempC <= FAN_OFF_TEMP) fanOn = false;

    if (fanOn) {
      pwmToSet = map((int)(tempC * 100), (int)(MIN_TEMP * 100), (int)(MAX_TEMP * 100), PWM_MIN, PWM_MAX);
      pwmToSet = constrain(pwmToSet, PWM_MIN, PWM_MAX);
      fanRunning = true;
    }
  } else {
    pwmToSet = manualPWM;
    fanRunning = (manualPWM > 0);
  }

  analogWrite(MOTOR_PWM, pwmToSet);
  updateBarGraph(fanRunning ? pwmToSet : 0);
}

// === Set Motor Direction ===
void setDirection(bool isForward) {
  forward = isForward;
  digitalWrite(MOTOR_IN1, forward ? HIGH : LOW);
  digitalWrite(MOTOR_IN2, forward ? LOW : HIGH);
}

// === Temperature Reading ===
float readTemperatureC() {
  int adc = analogRead(TEMP_PIN);
  float resistance = (1023.0 / adc - 1.0) * 10000.0;
  float tempK = 1.0 / (log(resistance / 10000.0) / 3950.0 + 1.0 / 298.15);
  return tempK - 273.15;
}

// === LED Bargraph Update ===
void updateBarGraph(int pwmVal) {
  int segments = map(pwmVal, 0, 255, 0, 9);
  if (segments > 8) segments = 8;
  byte pattern = 0;

  for (int i = 0; i < segments; i++) {
    pattern |= (1 << i);
  }

  digitalWrite(SHIFT_LATCH, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, pattern);
  digitalWrite(SHIFT_LATCH, HIGH);
}
