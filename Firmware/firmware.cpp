#include <ESP32Servo.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SERVO_PIN 1
#define MIC_PIN   0   // MAX9814 OUT
#define SDA_PIN   8
#define SCL_PIN   9

// OLED
Adafruit_SSD1306 display(128, 64, &Wire);

// BH1750
BH1750 lightMeter;

// Servo
Servo myServo;

// Time for scheduled toggle
int targetHour = 18;   // 6 PM
int targetMinute = 00;
bool alreadyTriggeredToday = false;

// Clap detection variables
unsigned long lastClap = 0;
int clapThreshold = 900;   // you can tune this
int clapWindow = 300;      // ms window to catch a clap
bool clapDetected = false;

// Light switch state
bool switchOn = false;

void flipSwitch() {
  if (switchOn) {
    myServo.write(0);
    delay(350);
  } else {
    myServo.write(90);
    delay(350);
  }
  switchOn = !switchOn;
}

void setup() {
  Serial.begin(115200);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Servo
  myServo.attach(SERVO_PIN);

  // OLED init
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(1);
  display.setTextSize(1);

  // BH1750
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  // Startup servo position
  myServo.write(0);
  delay(500);
}

void loop() {

  // READ MICROPHONE
  int micValue = analogRead(MIC_PIN);
  if (micValue > clapThreshold) {
    unsigned long now = millis();
    if (now - lastClap > clapWindow) {
      flipSwitch();  // clap → toggle
      lastClap = now;
    }
  }

  // LIGHT SENSOR
  float lux = lightMeter.readLightLevel();

  // TIME LOGIC (placeholder)
  // Until you add NTP: this triggers every loop at targetMinute second=0
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    if (timeinfo.tm_hour == targetHour &&
        timeinfo.tm_min == targetMinute &&
        !alreadyTriggeredToday) {
      flipSwitch();
      alreadyTriggeredToday = true;
    }
    if (timeinfo.tm_hour != targetHour) {
      alreadyTriggeredToday = false;
    }
  }

  // OLED DISPLAY
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Light: ");
  display.print(lux);
  display.println(" lx");

  display.print("Mic: ");
  display.println(micValue);

  display.print("Switch: ");
  display.println(switchOn ? "ON" : "OFF");

  display.display();

  delay(20);
}
