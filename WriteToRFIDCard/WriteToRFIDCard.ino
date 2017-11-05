#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

//Arduino Uno
//LCD
#define LCD_RS 8 
#define LCD_EN 7
#define LCD_D4 3
#define LCD_D5 4
#define LCD_D6 5
#define LCD_D7 6
//Motion sensor
#define Motion_sensor 14
//RFID
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); 

unsigned long uidDec, uidDecTemp; // hien thi so UID dang thap phan
int motionDetection = 0;
int i = 0;


void setup() {
    LiquidCrystal(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7); 
    Serial.begin(9600);    
    SPI.begin();                
    mfrc522.PCD_Init(); 
    pinMode(motionDetection, INPUT);

}

void loop() {
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) {
            key.keyByte[i] = 0xFF;
    }
        
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
    Serial.print("Card UID: ");
    Serial.println(uidDec);
    delay(3000);

    byte sector = 1;
    byte valueBlockA = 4;
    byte trailerBlock   = 7;
    byte status;
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
            Serial.print("PCD_Authenticate() failed: ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
    }


    Serial.println("Writing new value block A(4) : the first of the sector TWO ");
    byte value1Block[] = { 2,2,2,2,  2,2,2,2, 2,2,2,2,  2,2,2,2,   valueBlockA,~valueBlockA,valueBlockA,~valueBlockA };
    status = mfrc522.MIFARE_Write(valueBlockA, value1Block, 16);
    if (status != MFRC522::STATUS_OK) {
            Serial.print("MIFARE_Write() failed: ");
            Serial.println(mfrc522.GetStatusCodeName(status));
    }
    
                
    Serial.println("Read block A(4) : the first of the sector TWO");        
    byte buffer[18];
    byte size = sizeof(buffer);
    // change this: valueBlockA , for read anather block
    // cambiate valueBlockA per leggere un altro blocco
    status = mfrc522.MIFARE_Read(valueBlockA, buffer, &size);
    Serial.print("Ma Card:");
    for(i=0; i<=15; i++){
      Serial.print(buffer[i]);
    }
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    
}

