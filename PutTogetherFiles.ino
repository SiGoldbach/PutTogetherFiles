#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
//#include <MFRC522DriverI2C.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleDHT.h>
#include <SPI.h>



//RFID
#define RST_PIN         9
#define SS_PIN          10

MFRC522DriverPinSimple ss_pin(SS_PIN);

MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};  // Create MFRC522 instance.
void sendEntry();


//TEMP
String str;

int pinDHT11 = 2;
SimpleDHT11 dht11;


// Define a struct for the linked list node(Variable) to store temperature and humidity values
struct SensorData {
  float temperature;
  float humidity;
  SensorData* next;
};

//Decleration of function used for temperature measurements 
void printDataToSerial(SensorData* data);
void printDataToLCD(SensorData* data);
void tempLogic();
SensorData* sensorDataList = nullptr;

unsigned long previousMillis = 0;
const long interval = 15000;

// listening function, myserial listen
//Use write functions because i write from Arduino UNO to the ESP

//initialize the liquid crystal library
//the first parameter is  the I2C address
//the second parameter is for how many rows that are on screen 
//the  third parameter is for how many  columns that are on screen  
LiquidCrystal_I2C lcd(0x27,  16, 2);

void RC522_Init(unsigned long baudrate){
  Serial.begin(baudrate);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(40);
}

void setup() {
  RC522_Init(115200);
  lcd.init();
  // turn on the backlight
  lcd.backlight();
}

void loop() {
    sendEntry();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    tempLogic();
  }

}

void tempLogic(){
   // Update the last time the sensor was read


    byte temperature = 0;
    byte humidity = 0;
    byte data[40] = {0};

    // Read data from the DHT11 sensor
    if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
      Serial.print("Read DHT11 failed");
      return;
    }
    
    // Allocate memory for new sensor data
    SensorData* newSensorData = new SensorData;

    // Store temperature and humidity values in the new node
    newSensorData->temperature = (float)temperature;
    newSensorData->humidity = (float)humidity;
    //Output operations first to serial the to lcd. 
    printDataToSerial(newSensorData);
    printDataToLCD(newSensorData);
    // Update the linked list by adding the new node to the beginning
    newSensorData->next = sensorDataList;
    sensorDataList = newSensorData;


  // tell the screen to write on the top row
  
}


void sendEntry(){
  //Serial.println("Trying to read card");
  if(readAnyCard()){
    Serial.print("Door opened:");
    char* id = (char*)getLastId();
    for(int i = 0; i < 10; i++){
      if ((uint8_t)id[i] > 15){
        Serial.print((uint8_t)id[i], HEX);
      }
      else {
        Serial.print("0");
        Serial.print((uint8_t)id[i], HEX);
      }
    }
    Serial.print("\n");
  }

}



// Can return true many times in a row
bool matchLast(byte newCard[]){
  byte* readCard = getLastId();
  bool matched = true;
  for(int i = 0; i < 10; i++){
    if(readCard[i] != newCard[i]) matched = false;
  }
  return matched;
}

// Can only return true once per read
bool matchCurrent(byte newCard[]){
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  if (!mfrc522.PICC_ReadCardSerial()){
    return false;
  }
  mfrc522.PICC_HaltA();  
  for(int i = 0; i < 10; i++){
    if(mfrc522.uid.uidByte[i] != newCard[i]) return false;
  }
  return true;
}

// Returns true if any card is read
bool readAnyCard(){
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  if (!mfrc522.PICC_ReadCardSerial()){
    return false;
  }
  mfrc522.PICC_HaltA();  
  return true;
}

// Returns the ID of the last card that was read
byte* getLastId(){
  //mfrc522.PICC_IsNewCardPresent();
  //mfrc522.PICC_ReadCardSerial();
  //mfrc522.PICC_HaltA();
  return mfrc522.uid.uidByte;
}

//Method for serial communication to the esp8266 
void printDataToSerial(SensorData* data){
  Serial.print("Temperature: ");
    Serial.print(data->temperature);
    Serial.print(" *C, ");
    Serial.print("Humidity: ");
    Serial.print(data->humidity);
    Serial.println(" %");

}
//Method for writing on the lcd screen 
void printDataToLCD(SensorData* data){
    lcd.setCursor(0,0);
  // tell the screen to write "Temperature" and show the temperature on the top  row
  lcd.print("Temp: ");  
  lcd.print(data->temperature); lcd.print(" C");
  // tell the screen to write on the bottom  row
  lcd.setCursor(0,1);
  // tell the screen to write “IOT”  on the bottom row
  lcd.print("Humidity:"); 
  lcd.print(data->humidity); lcd.print(" %"); 
}


