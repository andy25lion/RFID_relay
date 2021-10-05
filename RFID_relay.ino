/*
 * --------------------------------------------------------------------------------------------------------------------
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>
//#include <RFID.h>
#define UID_SIZE 4

constexpr uint8_t RST_PIN = 9;          // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;         // Configurable, see typical pin layout above
constexpr uint8_t RELAY_PIN = A0;

uint8_t successRead;            // Variable integer to keep if we have Successful Read from Reader
uint8_t lastValidStatus = 0;     // Save last valid read status so we don't switch too often
uint8_t readCard[UID_SIZE];               // Stores scanned ID read from RFID Module

struct UID {
  uint8_t uid[UID_SIZE];
  uint8_t enabled;
};

//////////////////////// UIDs WITH "ENABLED" BYTE 1 WILL OPEN THE RELAY ///////////////////////////
UID targetCards[] =  {
  { {0x79, 0x2D, 0x8E, 0x9C}, 0 },
  { {0x4A, 0xD5, 0xCF, 0x81}, 0 },

  { {0x1A, 0xA0, 0xDD, 0x81}, 0 },
  { {0x89, 0x4C, 0x8F, 0x9C}, 0 },

  { {0x5A, 0x17, 0xE0, 0x81}, 1 },
  { {0x89, 0xE4, 0xB1, 0xB2}, 1 }
};
//////////////////////////////////////////////////////////////////////////////////////////

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
//RFID RC522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin(); 
  mfrc522.PCD_Init();   // Init MFRC522
//  RC522.init();
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  Serial.print("Running...\n");
}

void loop() {
  // Look for new cards
  successRead = getID();
  if(successRead) {
    Serial.print("Card detected!\n");
    if(checkUID()){
      if(!lastValidStatus) {
        Serial.print("Valid card, opening...\n");
        digitalWrite(RELAY_PIN, LOW);
        delay(200);
        digitalWrite(RELAY_PIN, HIGH);
        lastValidStatus = 1;
      }
    } else {
      Serial.print("Invalid card\n");
      lastValidStatus = 0;
    }
  } else {
    lastValidStatus = 0;
  }
//  Serial.println(".");
  delay(200);   
}

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Uncomment to reset the reader at every check
  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
//  Serial.println("Checking card");
  
//  uint8_t c = RC522.isCard(); 
  uint8_t c = mfrc522.PICC_IsNewCardPresent();
//  Serial.println(c);
  if ( ! c) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  Serial.println("Reading card");
//  if ( ! RC522.readCardSerial()) {   //Since a PICC placed get Serial and continue
  if ( ! mfrc522.PICC_ReadCardSerial()) { 
    return 0;
  }
  Serial.print("Scanned PICC's UID: ");
  for ( uint8_t i = 0; i < UID_SIZE; i++) {
//    readCard[i] = RC522.serNum[i];
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading (no use to read the entire card)
  return 1;
}

///////////////////////// Check for matching UIDs /////////////////////////////////
uint8_t checkUID() {
  uint8_t sizec = sizeof(targetCards) / sizeof(UID);//(UID_SIZE + 1);
  for (uint8_t i = 0; i < sizec; i++){
    if (targetCards[i].enabled == 1 && memcmp(readCard, targetCards[i].uid, UID_SIZE) == 0)
      return 1;
  }
  return 0;
}
///////////////////////////////////////////////////////////////////////////////////
