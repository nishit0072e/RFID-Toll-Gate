#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED reset pin (set to -1 if not used)
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER 0 // Buzzer pin (connect INPUT pin of buzzer to GPIO-0)

int ledAccessPin=22; //Green led
int ledDenyPin=24; //Red led

SPIClass SPI(0);
#define CH 0
TwoWire Wire(1);

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
Servo myServo;                   // Define servo name

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize SPI and MFRC522
  SPI.begin();
  mfrc522.PCD_Init();

  // Initialize servo
  myServo.attach(CH); // Servo pin
  myServo.write(0);   // Servo start position

  // Initialize buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW); // Ensure buzzer is off

  //Initialize led's
  pinMode(ledAccessPin, OUTPUT);
  digitalWrite(ledAccessPin, HIGH);
  pinMode(ledDenyPin, OUTPUT);
  digitalWrite(ledDenyPin, HIGH);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is the I2C address for the OLED
    Serial.println(F("SSD1306 initialization failed"));
    for (;;)
      ; // Stop here if OLED initialization fails
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Put card on the"));
  display.println(F("reader to check"));
  display.display();
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  // Clear the OLED display for new data
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("UID tag:"));

  // Read UID and display it
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    display.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    display.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  display.display();

  // Convert UID to uppercase for comparison
  content.toUpperCase();

  // Check if UID matches the authorized UID
  display.setCursor(0, 32);
  if (content.substring(1) == "63 15 52 A9") { // Replace with your authorized card's UID
    display.println(F("Access Granted"));
    digitalWrite(BUZZER, HIGH); // Turn on buzzer
    delay(300);
    digitalWrite(BUZZER, LOW);  // Turn off buzzer
    digitalWrite(ledAccessPin, LOW);
    myServo.write(90);          // Open servo
    display.display();
    delay(5000);                // Wait for 5 seconds
    myServo.write(0);           // Close servo
    digitalWrite(ledAccessPin, HIGH);
  } else {
    display.println(F("Access Denied"));
    digitalWrite(BUZZER, HIGH); // Turn on buzzer
    digitalWrite(ledDenyPin, LOW);
    delay(2000);
    digitalWrite(BUZZER, LOW);  // Turn off buzzer
    digitalWrite(ledDenyPin, HIGH);
  }
  display.display();

  delay(1000); // Wait 2 seconds before scanning the next card
}
