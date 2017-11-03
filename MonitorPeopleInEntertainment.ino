#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); 

unsigned long uidDec, uidDecTemp; // hien thi so UID dang thap phan
byte bCounter, readBit;
unsigned long ticketNumber;


void setup() {
    Serial.begin(9600);    
    SPI.begin();                
    mfrc522.PCD_Init(); 

}

void loop() {
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    uidDec = 0;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidDecTemp = mfrc522.uid.uidByte[i];
      uidDec = uidDec*256+uidDecTemp;
    } 
    if(uidDec == 3847333552){
      Serial.println("The nguoi lon");
    }
    if(uidDec == 1964994619){
      Serial.println("The tre em");
    }
    Serial.print("Card UID: ");
    Serial.println(uidDec);
    delay(3000);
    
}

