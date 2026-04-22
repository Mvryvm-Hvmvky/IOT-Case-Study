// Updated SmartLockerSystem.ino Code

#include <SPI.h>

// Pin Definitions
#define SCK_PIN 13
#define MISO_PIN 12
#define MOSI_PIN 11

void setup() {
    // Initialize SPI
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);
    // Other setup code...
}

void loop() {
    // Loop code...
    showMessage(); // Corrected from showMeIssage
}