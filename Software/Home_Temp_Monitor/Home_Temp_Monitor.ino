#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <DHT22.h>

// lcd initialization
hd44780_I2Cexp lcd;
// end lcd initialization

// dht22 sensor initialization
#define pinDHT 8

DHT22 dht22(pinDHT);
// end dht22 sensor initialization

// Global variables

//Interrupt function wake up button
const byte wakeUp = 2;

// Display LCD
int cols = 16;
int rows = 2;
int onTime = 0; // start at 0 millis onTime
volatile bool lcdState = false; // state of the lcd when NOT backlit
volatile bool dhtState = true; // initial state of the sensor is good
int errorCounter = 0;

// DHT22 sensor
float temp;
int errorTemp = 5; // 5 degrees over measured temperature by other termomether
float humidity;
int errorHumidity = 0; // humidity pretty accurate

// SPI Settings
const int spiSpeed = 4000000;

// SD card
#define DATA_DIRECTORY "MEAS" // limit is 8 characters
#define RECORD_FILE_EXT ".CSV"
File sensorData;


void setup() {
  // put your setup code here, to run once:

// serial
Serial.begin(115200);

while(!Serial){
  // waits for serial port to connect
}

Serial.println("Serial Port Connected...");
// spi
SPI.begin();
SPISettings mySettings(spiSpeed,MSBFIRST,SPI_MODE0);
// sd

if(SD.begin()){
  Serial.println("SD card connected!");
} else {
  Serial.println("No SD card!");
}

SD.mkdir(DATA_DIRECTORY);
sensorData = SD.open("data.csv",FILE_WRITE);
sensorData.write("New meas set");
sensorData.close();



// lcd
lcd.begin(rows,cols);
lcd.setBacklight(0);

// interrupt 
pinMode(wakeUp,INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(wakeUp),wakeLCD,FALLING);

}

void loop() {
  // put your main code here, to run repeatedly:

temp = dht22.getTemperature() - errorTemp;
humidity = dht22.getHumidity() - errorHumidity;
delay(2000);

 if(lcdState == true && millis() - onTime > 10000 ){
  lcd.setBacklight(0);
  lcdState = false;
}

if(humidity == 0 || temp == -278 || temp == -5){ // one of the states DHT22 function reads if no communication with DHT sensor
 Serial.println("Error entered");
 errorCounter = errorCounter + 1; 

  while(errorCounter != 0){
    Serial.println(errorCounter);
    Serial.print("Temperature: ");
    Serial.println(temp);

    Serial.print("Humidity: ");
    Serial.println(humidity);
    if(errorCounter > 8){ // errorCounter cannot be more than 10
      errorCounter = 8;
    }

    if(temp == -5 || humidity == 0 || temp == -278){
      errorCounter++;
      Serial.println("Counter incremented");
      delay(500);
    } 

    if(temp != -278 && temp != -5 && humidity != 0){
      errorCounter--;
      Serial.println("Counter decremented");
      delay(1000);
    }

    lcd.clear();
    lcd.print("Sensor data not");
    lcd.setCursor(0,1);
    lcd.print("available Err(");
    lcd.print(errorCounter);
    lcd.print(")");

      // read again sensor data, obviously
      delay(2000); 
    temp = dht22.getTemperature() - errorTemp; 
    humidity = dht22.getHumidity() - errorHumidity;
    Serial.println(temp);

  }

}


lcd.clear();
lcd.print("Temp:");
lcd.print(temp);
lcd.print(" deg");
lcd.setCursor(0,1);
lcd.print("Humidity:");
lcd.print(humidity);
lcd.print(" %");


// Serial.print("Good Temperature: ");
// Serial.println(temp);

// Serial.print("Good Humidity: ");
// Serial.println(humidity);


}

void wakeLCD (){

  if(lcdState == false){
    lcdState = true;
    lcd.setBacklight(1);
    onTime = millis();
  }
}