#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

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
#define INFRARED_PIN 15
//RFID
#define SS_PIN 10
#define RST_PIN 9
//SPEAKER
#define SPEAKER 2
//Relay
#define RELAY 16
//RFID
MFRC522 mfrc522(SS_PIN, RST_PIN); 

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
//Variable of RFID Card
byte buffer[18];
byte sector = 1;
byte valueBlockA = 4;
byte trailerBlock   = 7;
byte status;
unsigned long uidDec, uidDecTemp; // hien thi so UID dang thap phan
//
int motionDetection = 0;
int cardType = 0;
int higherThan1_2m = 0;
int warning = 0;
int numberOfPeople = 0;

void checkCardNumber(void);
void cardProcess(void);
void dislayLCD(void);
void setup() {
    LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7); 
    Serial.begin(9600);    
    SPI.begin();                
    mfrc522.PCD_Init(); 
    pinMode(MOTION_PIN, INPUT);
    pinMode(SPEAKER, OUTPUT);
    pinMode(RELAY, OUTPUT);
    digitalWrite(RELAY, LOW);
    lcd.begin(20,4);
    dislayLCD();
}

void loop() {
    higherThan1_2m = digitalRead(INFRARED_PIN);
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
    /*
    Serial.print("Card UID: ");
    Serial.println(uidDec);
    delay(3000);
    */
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
            Serial.print("PCD_Authenticate() failed: ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
    }        
    byte size = sizeof(buffer);
    status = mfrc522.MIFARE_Read(valueBlockA, buffer, &size);
    checkCardNumber();
    cardProcess();
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();  
}


void checkCardNumber(void){ 
  if (buffer[0]  == 1 && buffer[1]  == 1 && buffer[2]  == 1 && buffer[3]  == 1 && buffer[4]  == 1 && buffer[5]  == 1 && buffer[6]  == 1 && buffer[7]  == 1 && 
     buffer[8]  == 1 && buffer[9]  == 1 && buffer[10] == 1 && buffer[11] == 1 && buffer[12] == 1 && buffer[13] == 1 && buffer[14] == 1 && buffer[15] == 1 )
     {
        cardType = ADULT;
        digitalWrite(SPEAKER,HIGH);
        delay(200);
        digitalWrite(SPEAKER,LOW); 
     }
  if (buffer[0]  == 2 && buffer[1]  == 2 && buffer[2]  == 2 && buffer[3]  == 2 && buffer[4]  == 2 && buffer[5]  == 2 && buffer[6]  == 2 && buffer[7]  == 2 && 
     buffer[8]  == 2 && buffer[9]  == 2 && buffer[10] == 2 && buffer[11] == 2 && buffer[12] == 2 && buffer[13] == 2 && buffer[14] == 2 && buffer[15] == 2 )
     {
        cardType = CHILD;
        digitalWrite(SPEAKER,HIGH);
        delay(200);
        digitalWrite(SPEAKER,LOW); 
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

