// 4-bit ripple counter password system using Arduino as clock

const int qPins[4] = {2, 3, 4, 5};   // Q0..Q3 (LSB..MSB)
const int clockPin = 8;              // Arduino -> clock input of first 7473
const int ledGreen = 10;
const int ledRed   = 11;

byte PASSWORD = 0b1010; // default 4-bit password (change as needed)
unsigned long lastToggle = 0;
unsigned long clockInterval = 1000; // ms between clock pulses (change to speed up/slow down)
bool clockState = LOW;
unsigned long pulseWidth = 10; // ms clock HIGH pulse width

void setup() {
  Serial.begin(9600);
  Serial.println("4-bit counter with Arduino clock. Default password = 1010");

  for (int i = 0; i < 4; i++) pinMode(qPins[i], INPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  digitalWrite(clockPin, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, LOW);
}

byte readBits() {
  byte v = 0;
  for (int i = 0; i < 4; i++) {
    v |= (digitalRead(qPins[i]) << i); // LSB at index 0
  }
  return v;
}

void checkSerialInput() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 4) {
      byte newPass = 0;
      bool valid = true;
      for (int i = 0; i < 4; i++) {
        if (input[i] == '0') newPass = (newPass << 1);
        else if (input[i] == '1') newPass = (newPass << 1) | 1;
        else valid = false;
      }
      if (valid) {
        PASSWORD = newPass;
        Serial.print("Password updated to: ");
        Serial.println(input);
      } else Serial.println("Invalid input! Enter 4-bit binary e.g., 1010");
    } else Serial.println("Invalid length! Enter 4-bit binary e.g., 0110");
  }
}

void loop() {
  checkSerialInput();

  // generate a periodic clock pulse: LOW -> HIGH (pulseWidth ms) -> LOW
  unsigned long now = millis();

  if (!clockState && (now - lastToggle >= clockInterval)) {
    // raise clock
    digitalWrite(clockPin, HIGH);
    clockState = HIGH;
    lastToggle = now;
  } else if (clockState && (now - lastToggle >= pulseWidth)) {
    // lower clock
    digitalWrite(clockPin, LOW);
    clockState = LOW;
    lastToggle = now;

    // after falling edge, read counter (ripple counters often update on falling edge)
    byte val = readBits();
    Serial.print("Counter = ");
    Serial.println(val, BIN);

    if (val == PASSWORD) {
      Serial.println("✅ Correct Password!");
      digitalWrite(ledGreen, HIGH);
      digitalWrite(ledRed, LOW);
      delay(3000);
      digitalWrite(ledGreen, LOW);
    } else {
      Serial.println("❌ Wrong Password");
      digitalWrite(ledRed, HIGH);
      delay(250);
      digitalWrite(ledRed, LOW);
    }
  }
}