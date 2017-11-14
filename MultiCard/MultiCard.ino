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
#define LCD_RS 8 
#define LCD_EN 7
#define LCD_D4 6
#define LCD_D5 5
#define LCD_D6 4
#define LCD_D7 3
//Motion sensor
#define MOTION_PIN 14
//Infrared_Sensor
#define INFRARED_PIN_0 15
//RFID
#define RST_PIN 9     
#define SS_1_PIN 10   
#define SS_2_PIN 8
#define NR_OF_READERS 2
//SPEAKER
#define SPEAKER 2
//Relay
#define RELAY 16
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
//
int motionDetection = 0;
int cardType = 0;
int higherThan1_2m[NR_OF_READERS];
int warning = 0;
int numberOfPeople = 0;

void checkCardNumber(void);
void cardProcess(void);
void dislayLCD(void);

void setup() {
  LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
  pinMode(MOTION_PIN, INPUT);
  pinMode(SPEAKER, OUTPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  Serial.begin(9600); 
  SPI.begin();        
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
  lcd.begin(20,4);
  dislayLCD();
}

/**
 * Main loop.
 */
void loop() {
  higherThan1_2m[0] = digitalRead(INFRARED_PIN_0);
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards
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
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));

      status = mfrc522[reader].PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522[reader].uid));
      if (status != MFRC522::STATUS_OK) {
              Serial.print("PCD_Authenticate() failed: ");
              Serial.println(mfrc522[reader].GetStatusCodeName(status));
              return;
      }        
      byte size = sizeof(buffer);
      status = mfrc522[reader].MIFARE_Read(valueBlockA, buffer, &size);
      checkCardNumber();
      cardProcess();
      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
      delay(1000);
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
  if (buffer[0]  == 2 && buffer[1]  == 2 && buffer[2]  == 2 && buffer[3]  == 2 && buffer[4]  == 2 && buffer[5]  == 2 && buffer[6]  == 2 && buffer[7]  == 2 && 
     buffer[8]  == 2 && buffer[9]  == 2 && buffer[10] == 2 && buffer[11] == 2 && buffer[12] == 2 && buffer[13] == 2 && buffer[14] == 2 && buffer[15] == 2 )
     {
        cardType = CHILD;
        Serial.println("Child");
     }   
}

void cardProcess(void){
  if(cardType == ADULT){
    lcd.print("THE NGUOI LON");
    warning = 0;
    numberOfPeople++;
    dislayLCD();
    lcd.setCursor(4,2);
    lcd.print("NGUOI LON");
    lcd.setCursor(0,3);
    lcd.print("THE HOP LE");
    digitalWrite(RELAY, HIGH);
    delay(2000);
    digitalWrite(RELAY, LOW);
  }
  if(cardType == CHILD && higherThan1_2m == YES){
      dislayLCD();
      lcd.setCursor(4,2);
      lcd.print("TRE EM");
      lcd.setCursor(0,3);
      lcd.print("THE KHONG HOP LE");
      warning++;
  }
  if(cardType == CHILD && higherThan1_2m == NO){
      warning = 0;
      numberOfPeople++;
      dislayLCD();
      lcd.setCursor(4,2);
      lcd.print("TRE EM");
      lcd.setCursor(0,3);
      lcd.print("THE HOP LE");
      digitalWrite(RELAY, HIGH);
      delay(2000);
      digitalWrite(RELAY, LOW);
  }
  if(warning == 3){
      digitalWrite(SPEAKER, HIGH);
      delay(2000);
      digitalWrite(SPEAKER, LOW);
      warning = 0;
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
