#include <LiquidCrystal.h>
#include <stdio.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

int ir1 = A0;
int ir2 = A1;
int vib = A3;
int buzzer = A4;
int sti = 0;
String inputString = "";
boolean stringComplete = false;

float voltage1 = 0;
float weight = 0.0;
int cntlmk = 0;

// Calibration constants
#define SENSOR_OFFSET     0.76
#define SENSOR_SCALE      4.32
#define WEIGHT_TARE_GRAMS 1000.0
#define WEIGHT_MAX_GRAMS  5000.0

void beep() {
  digitalWrite(buzzer, LOW);
  delay(2000);
  digitalWrite(buzzer, HIGH);
}

// Display an integer (vehicle count) on LCD at current cursor
void convertl1(int value) {
  if (value < 0) value = 0;
  char buf[6];
  snprintf(buf, sizeof(buf), "%-5d", value);  // left-aligned, 5 chars wide
  lcd.print(buf);
}

// Display weight (float, in grams) on LCD at current cursor
void convertl(float value) {
  if (value < 0) value = 0;
  char buf[8];
  // Display as integer grams, 6 chars wide
  snprintf(buf, sizeof(buf), "%-6d", (int)value);
  lcd.print(buf);
}

// Send a value over Serial as ASCII digits and also display last 3 digits on LCD
void converts(unsigned int value) {
  unsigned int a, b, c, d, e, f, g, h;
  a = value / 10000;
  b = value % 10000;
  c = b / 1000;
  d = b % 1000;
  e = d / 100;
  f = d % 100;
  g = f / 10;
  h = f % 10;

  a = a | 0x30;
  c = c | 0x30;
  e = e | 0x30;
  g = g | 0x30;
  h = h | 0x30;

  // Send all 5 digits over Serial
  Serial.write(a);
  Serial.write(c);
  Serial.write(e);
  Serial.write(g);  // Fixed: was missing
  Serial.write(h);  // Fixed: was missing

  // Display last 3 digits on LCD
  lcd.write(e);
  lcd.write(g);
  lcd.write(h);
}

void setup() {
  Serial.begin(9600);
  // Removed incorrect serialEvent() call here

  pinMode(ir1, INPUT);
  pinMode(ir2, INPUT);
  pinMode(vib, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Bridge Watch");
  delay(1500);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("C:");

  lcd.setCursor(8, 0);
  lcd.print("V:");

  lcd.setCursor(0, 1);
  lcd.print("Wt:");
}

void loop() {
  // IR Sensor 1 — increment vehicle count
  if (digitalRead(ir1) == LOW) {
    delay(200);
    while (digitalRead(ir1) == LOW);
    cntlmk++;
  }

  // IR Sensor 2 — decrement vehicle count
  if (digitalRead(ir2) == LOW) {
    delay(200);
    while (digitalRead(ir2) == LOW);
    cntlmk--;
    if (cntlmk <= 0) cntlmk = 0;
  }

  // Display vehicle count
  lcd.setCursor(2, 0);
  convertl1(cntlmk);

  // Vibration sensor
  if (digitalRead(vib) == LOW) {
    lcd.setCursor(10, 0);
    lcd.print("ON ");
    beep();
  } else {
    lcd.setCursor(10, 0);
    lcd.print("OFF");
  }

  // Read weight from load cell via A5
  voltage1 = analogRead(A5);
  voltage1 = voltage1 * (5.0 / 1023.0);
  weight = ((voltage1 - SENSOR_OFFSET) * SENSOR_SCALE) * 1000.0;  // grams

  // Tare: subtract baseline
  if (weight <= WEIGHT_TARE_GRAMS) {
    weight = 0;
  } else {
    weight = weight - WEIGHT_TARE_GRAMS;
  }

  // Display weight
  lcd.setCursor(3, 1);
  convertl(weight);

  // Overload alert
  if (weight > WEIGHT_MAX_GRAMS) {
    beep();
  }

  delay(100);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    sti++;
    if (sti == 12) {
      sti = 0;
      stringComplete = true;
    }
  }
}