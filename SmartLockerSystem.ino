//==== Libraries ====
#include <SPI.h>
#include <MFRC522.h>
#include <Firebase.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <UbidotsESPMQTT.h>

//==== Macros ====
#define SS_PIN 2
#define RST_PIN 5
#define MISO_PIN 19
#define MOSI_PIN 23
#define SCK_PIN 18
#define WIFI_SSID "MaryamLaptop"
#define WIFI_PASSWORD "12345678"
#define REFERENCE_URL "https://test-project-c546e-default-rtdb.firebaseio.com/"
#define AUTH_TOKEN "AIzaSyBruJ1_aW6fTZUB0_DZtDWqaAmPmo0LkRA"
#define G_LED_PIN 12
#define R_LED_PIN 14
#define BUZZER_PIN 27
#define SCREEN_WIDTH 128                             // OLED display width, in pixels
#define SCREEN_HEIGHT 64                             // OLED display height, in pixels
#define OLED_RESET -1                                // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C                          ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define TOKEN "BBUS-t4BMJ9LwhN4IX4gR3KmTpH50bfoL2t"  // Your Ubidots TOKEN
#define DEVICE_LABEL "esp32"
#define VAR_LABEL "lockernumber"

//==== Instances ====
MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
Firebase fb(REFERENCE_URL, AUTH_TOKEN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Ubidots client(TOKEN);

//==== Methods ====
String readUID() {
  String uid = "";
  byte* buffer = rfid.uid.uidByte;
  byte bufferSize = rfid.uid.size;
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) uid += "0";
    uid += String(buffer[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void shortBeep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
}

void longBeep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
}

void showMessage(String message) {
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 20);             // Start at top-left corner
  display.println(message);
  delay(500);
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(G_LED_PIN, OUTPUT);
  pinMode(R_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  /* Connect to WiFi */
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("-");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println();

  //====Initalization====
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522
  client.begin(callback);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}

void loop() {
  // put your main code here, to run repeatedly:
  client.loop();

  //==== Read RFID tag ====
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  String uid = readUID();
  Serial.println("Tag UID: " + uid);

  //==== Get locker number from RTDB ====
  String path = "/lockers/" + uid;
  int lockerNumber = -1;
  fb.getInt(path, lockerNumber);

  //==== Core logic ====
  if (lockerNumber > 0) {
    showMeIssage("Locker number: " + String(lockerNumber));
    digitalWrite(G_LED_PIN, HIGH);
    shortBeep();
    digitalWrite(G_LED_PIN, LOW);
    client.add(VAR_LABEL, lockerNumber);
    client.ubidotsPublish(DEVICE_LABEL);
  } else {
    showMessage("Access Denied");
    digitalWrite(R_LED_PIN, HIGH);
    longBeep();
    digitalWrite(R_LED_PIN, LOW);
  }

  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
