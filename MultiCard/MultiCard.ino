#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Debug.h>
#include <MFRC522Extended.h>
#include <MFRC522Hack.h>
#include <require_cpp11.h>
#include <LiquidCrystal.h>
#include <SPI.h>

//Arduino Uno Pins
//LCD
#define LCD_RS 7 
#define LCD_EN 6
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2
//Motion sensor
#define MOTION_PIN 14
//Infrared_Sensor
#define INFRARED_PIN_1 14
#define INFRARED_PIN_2 15
//RFID
#define RST_PIN 9     
#define SS_1_PIN 10   
#define SS_2_PIN 8
#define NR_OF_READERS 2
//SPEAKER
#define SPEAKER_1 18
#define SPEAKER_2 19
//Relay
#define RELAY_1 16
#define RELAY_2 17
//Card
#define ADULT 1
#define CHILD 2
//Motion
#define MOTION_UNDETECTED 0
#define MOTION_DETECTED 1
//infraredSensor
#define YES 0
#define NO 1

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
//Variable of RFID
byte ssPins[] = {SS_1_PIN, SS_2_PIN};
MFRC522 mfrc522[NR_OF_READERS];
byte buffer[18];
byte sector = 1;
byte valueBlockA = 4;
byte trailerBlock = 7;
byte status;
uint8_t reader = 0;
//
int motionDetection = 0;
int cardType = 0;
int higherThan1_2m[NR_OF_READERS];
int warning[2];
int numberOfPeople = 0;
int i = 0;
void checkCardNumber(void);
void cardProcess(void);
void dislayLCD(void);
void turnOnRelay(int numOfRelay);
void turnOffRelay(int numOfRelay);
void turnOnSpeaker(int numOfSpeaker);
void turnOffSpeaker(int numOfSpeaker);



void setup() {
  LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
  pinMode(INFRARED_PIN_1, INPUT);
  pinMode(INFRARED_PIN_2, INPUT);
  pinMode(SPEAKER_1, OUTPUT);
  pinMode(SPEAKER_2, OUTPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  Serial.begin(9600); 
  SPI.begin();        
  for (reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
  lcd.begin(20,4);
  dislayLCD();
}

void loop() {
  higherThan1_2m[0] = digitalRead(INFRARED_PIN_1);
  higherThan1_2m[1] = digitalRead(INFRARED_PIN_2);
  for (reader = 0; reader < NR_OF_READERS; reader++) {
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) {
       key.keyByte[i] = 0xFF;
    }
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()){
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      status = mfrc522[reader].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[reader].uid));
      if(status != MFRC522::STATUS_OK){
              Serial.print("PCD_Authenticate() failed: ");
              Serial.println(mfrc522[reader].GetStatusCodeName(status));
              return;
      }        
      byte size = sizeof(buffer);
      status = mfrc522[reader].MIFARE_Read(valueBlockA, buffer, &size);
      checkCardNumber();
      

      /*
      //Xoa Card
      if(cardType == ADULT || cardType == CHILD){
        delay(500);
        Serial.println("Chuan bi xoa card");
        byte value1Block[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,valueBlockA,~valueBlockA,valueBlockA,~valueBlockA};
        status = mfrc522[reader].MIFARE_Write(valueBlockA, value1Block, 16);
        if(status != MFRC522::STATUS_OK) {
                Serial.print("Xoa card that bai. Vui long thu lai");
        }
        else{           
          Serial.println("Xoa Card thanh cong");        
        }
      }
      */
      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
      turnOnSpeaker(reader);
      delay(200);
      turnOffSpeaker(reader);
      delay(2000);
      cardProcess();
    }
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void checkCardNumber(void){ 
  if (buffer[0]  == 1 && buffer[1]  == 1 && buffer[2]  == 1 && buffer[3]  == 1 && buffer[4]  == 1 && buffer[5]  == 1 && buffer[6]  == 1 && buffer[7]  == 1 && 
     buffer[8]  == 1 && buffer[9]  == 1 && buffer[10] == 1 && buffer[11] == 1 && buffer[12] == 1 && buffer[13] == 1 && buffer[14] == 1 && buffer[15] == 1 )
     {
        cardType = ADULT;
        Serial.println("Adult");
     }
  else if (buffer[0]  == 2 && buffer[1]  == 2 && buffer[2]  == 2 && buffer[3]  == 2 && buffer[4]  == 2 && buffer[5]  == 2 && buffer[6]  == 2 && buffer[7]  == 2 && 
     buffer[8]  == 2 && buffer[9]  == 2 && buffer[10] == 2 && buffer[11] == 2 && buffer[12] == 2 && buffer[13] == 2 && buffer[14] == 2 && buffer[15] == 2 )
     {
        cardType = CHILD;
        Serial.println("Child");
     }
  else {
        cardType = 0;
        Serial.println("The bi loi hoac da bi xoa. Vui long lien he to ky thuat");
  }
}


void cardProcess(void){
  if(cardType == ADULT){
    lcd.print("THE NGUOI LON");
    warning[reader] = 0;
    numberOfPeople++;
    dislayLCD();
    lcd.setCursor(4,2);
    lcd.print("NGUOI LON");
    lcd.setCursor(0,3);
    lcd.print("THE HOP LE");
    turnOnRelay(reader);
    delay(2000);
    turnOffRelay(reader);
  }
  if(cardType == CHILD && higherThan1_2m == YES){
      dislayLCD();
      lcd.setCursor(4,2);
      lcd.print("TRE EM");
      lcd.setCursor(0,3);
      lcd.print("THE KHONG HOP LE");
      warning[reader]++;
  }
  if(cardType == CHILD && higherThan1_2m == NO){
      warning[reader] = 0;
      numberOfPeople++;
      dislayLCD();
      lcd.setCursor(4,2);
      lcd.print("TRE EM");
      lcd.setCursor(0,3);
      lcd.print("THE HOP LE");
      turnOnRelay(reader);
      delay(2000);
      turnOffRelay(reader);
  }
  if(warning == 3){
      turnOnSpeaker(reader);
      delay(2000);
      turnOffSpeaker(reader);
      warning[reader] = 0;
  }
}

void dislayLCD(void){
  lcd.begin(20,4);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("TONG SO KHACH:");
  lcd.setCursor(14,1);
  lcd.print(numberOfPeople,DEC);
  lcd.setCursor(0,2);
  lcd.print("THE:");
}

void turnOnRelay (int numOfRelay){
  if(numOfRelay == 0){
    digitalWrite(RELAY_1, HIGH);
  }
  if(numOfRelay == 1){
    digitalWrite(RELAY_2, HIGH);
  }
}

void turnOffRelay (int numOfRelay){
  if(numOfRelay == 0){
    digitalWrite(RELAY_1, LOW);
  }
  if(numOfRelay == 1){
    digitalWrite(RELAY_2, LOW);
  }
}

void turnOnSpeaker (int numOfSpeaker){
  if(numOfSpeaker == 0){
    digitalWrite(SPEAKER_1, HIGH);
  }
  if(numOfSpeaker == 1){
    digitalWrite(SPEAKER_2, HIGH);
  }
}

void turnOffSpeaker (int numOfSpeaker){
  if(numOfSpeaker == 0){
    digitalWrite(SPEAKER_1, LOW);
  }
  if(numOfSpeaker == 1){
    digitalWrite(SPEAKER_2, LOW);
  }
}
