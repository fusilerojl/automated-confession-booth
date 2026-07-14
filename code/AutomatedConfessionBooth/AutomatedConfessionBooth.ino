#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define pins (same as before)
#define BUTTON1_PIN 2
#define BUTTON2_PIN 3
#define BUTTON3_PIN 4
#define BUTTON_ACKNOWLEDGE_PIN 5

int pirPin = 7;
int ledPin = 6;
int Buzzer = 8;

int priestLedPin = 10;
int priestBuzzerPin = 11;

int statusGreenPin = 12;
int statusYellowPin = 13;
int statusRedPin = 9;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int calibrationTime = 30;
boolean penitentWaiting = false;
unsigned long confessionStartTime = 0;
boolean timerActive = false;

boolean personInBooth = false;
unsigned long lastMotionDetectedTime = 0;
const unsigned long motionTimeout = 10000; // 10 seconds of no motion = booth is vacant


void setup() {
    Serial.begin(9600);
    lcd.begin();
    lcd.backlight();
    lcd.print("   Automated");
    lcd.setCursor(0, 1);
    lcd.print("Confession Booth");
    delay(2000);
    lcd.clear();

    // Turn off the green LED before calibration
    digitalWrite(statusGreenPin, LOW);
    
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
    pinMode(BUTTON3_PIN, INPUT_PULLUP);
    pinMode(BUTTON_ACKNOWLEDGE_PIN, INPUT_PULLUP);

    pinMode(pirPin, INPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(Buzzer, OUTPUT);

    pinMode(priestLedPin, OUTPUT);
    pinMode(priestBuzzerPin, OUTPUT);
  
    pinMode(statusGreenPin, OUTPUT);
    pinMode(statusYellowPin, OUTPUT);
    pinMode(statusRedPin, OUTPUT);
    
    lcd.print("Calibrating");
    lcd.setCursor(0, 1);
    lcd.print("Sensor.....");
    Serial.print("Calibrating sensor ");
    for (int i = 0; i < calibrationTime; i++) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println(" done");
    lcd.clear();

    // Turn on the green LED after calibration is done
    digitalWrite(statusGreenPin, HIGH);

    lcd.print(" Sensor Active");
    lcd.clear();
    lcd.print(" Booth is Ready");
    Serial.println("SENSOR ACTIVE");
    delay(50);
}


void loop() {
  checkConfessionButtons();
  checkAcknowledgeButton();
  checkMotionAndBuzzer();
  handleTimer();
  delay(100);
}

void checkConfessionButtons() {
  if (digitalRead(BUTTON1_PIN) == HIGH) {
    startConfession("General Confession");
  }

  if (digitalRead(BUTTON2_PIN) == HIGH) {
    startConfession("Specific Confession");
  }

  // Button to conclude the confession
  if (digitalRead(BUTTON3_PIN) == HIGH) {
    concludeConfession();
  }
}

void startConfession(String confessionType) {
    notifyPriest(confessionType);
    setBoothStatus(1);
    penitentWaiting = true;
    timerActive = true;
    confessionStartTime = millis();
    
    lcd.clear();
    if (confessionType == "General Confession") {
        lcd.print("General Confess.");
        lcd.setCursor(0, 1);
        delay(2000); // Wait briefly before showing the next message

        // Display reflection prompts
        lcd.clear();
        lcd.print("Pause, reflect...");
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("On your journey...");
        delay(3000); // Display message for 3 seconds
        lcd.clear();
        delay(500);
        lcd.print("Think of gifts...");
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("And Trials....");
        delay(3000);
        lcd.clear();
        delay(500);
        lcd.print("Seek healing and...");
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("Open your heart...");
        delay(3000);
        lcd.clear();
        lcd.print("Please wait...");

    } else if (confessionType == "Specific Confession") {
        lcd.print("Specific Confession");
        lcd.setCursor(0, 1);
        delay(2000);

          lcd.clear();
          lcd.print("Focus on recent...");
          delay(1000);
          lcd.setCursor(0, 1);
          lcd.print("thoughts, deeds...");
          delay(3000); // Display message for 3 seconds
          lcd.clear();
          delay(500);
          lcd.print("Acknowledge your...");
          delay(1000);
          lcd.setCursor(0, 1);
          lcd.print("Problems, Sins...");
          delay(3000);
          lcd.clear();
          delay(500);
          lcd.print("Seek strength to...");
          delay(1000);
          lcd.setCursor(0, 1);
          lcd.print("Move forward....");
          delay(3000);
          lcd.clear();
          lcd.print("Please wait...");
    }
}

void checkAcknowledgeButton() {
  if (penitentWaiting && digitalRead(BUTTON_ACKNOWLEDGE_PIN) == HIGH) {
    setBoothStatus(2);
    digitalWrite(priestLedPin, HIGH);
    tone(priestBuzzerPin, 2200, 500);
    Serial.println("Confession ongoing");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Confession is");
    lcd.setCursor(0, 1);
    lcd.print("Ongoing......");
    delay(2000);
    digitalWrite(priestLedPin, LOW);
    penitentWaiting = false;
    timerActive = false; // Stop the timer
  }
}

void concludeConfession() {
  setBoothStatus(0); // Booth is now available
  penitentWaiting = false; // Reset waiting state
  timerActive = false; // Stop the timer
  tone(priestBuzzerPin, 2000, 1000);
  lcd.clear();
  lcd.print("Confession Done");
  Serial.println("Confession concluded. Booth is ready.");
  delay(2000); // Brief display for confirmation
  lcd.clear();
  lcd.print("Booth Available");
}

void checkMotionAndBuzzer() {
  static int lastPirState = LOW;
  int pirState = digitalRead(pirPin);

  if (pirState == HIGH && lastPirState == LOW) {
    digitalWrite(ledPin, HIGH);
    tone(Buzzer, 1500, 1000);
    Serial.println("Motion detected");
    lastPirState = HIGH;
  } else if (pirState == LOW && lastPirState == HIGH) {
    digitalWrite(ledPin, LOW);
    noTone(Buzzer);
    lastPirState = LOW;
  }
}


void handleTimer() {
  if (timerActive && (millis() - confessionStartTime >= 30000)) {
    // 30 seconds have passed, and no acknowledgment from the priest
    setBoothStatus(0); // Reset booth to available
    penitentWaiting = false;
    timerActive = false; // Stop the timer
    lcd.clear();
    lcd.print("Sorry, Priest is");
    lcd.setCursor (0,1);
    lcd.print("Unvailable now..");
    delay(2000);
    lcd.clear();
    lcd.print("Try again later..");
    delay(3000);
    Serial.println("Priest unavailable");
    delay(2000); // Display message briefly
    lcd.clear();
    lcd.print("Booth Available");
  }
}

void notifyPriest(String confessionType) {
  digitalWrite(priestLedPin, HIGH);
  tone(priestBuzzerPin, 1300, 800);
  Serial.print("Priest notified: ");
  Serial.println(confessionType);
}

void setBoothStatus(int status) {
  switch (status) {
    case 0:
      digitalWrite(statusGreenPin, HIGH);
      digitalWrite(statusYellowPin, LOW);
      digitalWrite(statusRedPin, LOW);
      break;
    case 1:
      digitalWrite(statusGreenPin, LOW);
      digitalWrite(statusYellowPin, HIGH);
      digitalWrite(statusRedPin, LOW);
      break;
    case 2:
      digitalWrite(statusGreenPin, LOW);
      digitalWrite(statusYellowPin, LOW);
      digitalWrite(statusRedPin, HIGH);
      break;
  }
}

void displayMessage(String message) {
  lcd.clear();
  lcd.print(message);
  delay(2000);
}
