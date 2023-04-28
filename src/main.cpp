/* Author: Homero
Device: Arduino 33 BLE Sense
Compatible with: Sense app for Android
*/

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Arduino_HS300x.h>

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

int data = -1;

BLEService dataService(deviceServiceUuid); 
BLECharacteristic dataCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLEWrite | BLEIndicate | BLENotify, 23);

unsigned long startMillis;
unsigned long currentMillis;
unsigned long period = 1000;

float old_temp = 0;
float old_hum = 0;

void setup() {
  Serial.begin(115200);
  if (!HS300x.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }
   if (!BLE.begin()) {
    Serial.println("* Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  BLE.setLocalName("Nano33BLE"); 
  BLE.setAdvertisedService(dataService);
  dataService.addCharacteristic(dataCharacteristic);
  BLE.addService(dataService);
  BLE.advertise();
  Serial.println("Arduino Nano 33 BLE Sense (Central Device)");
  Serial.println(BLE.address());
  Serial.println(" ");

  /* Set the clock time*/
  currentMillis = millis();
}


float readTemperature(){
  float temperature = HS300x.readTemperature();
  return temperature;
}

float readHumidity(){
  float humidity = HS300x.readHumidity();
  return humidity;
}



void bleController(){
  
/* Controls the data received to the smart device and decodes it*/
  BLEDevice central = BLE.central();
  Serial.println("- Discovering central device...");
  delay(500);
  if (central) {
    Serial.println("* Connected to central device!");
    Serial.print("* Device MAC address: ");
    Serial.println(central.address());
    Serial.println(" ");

    while (central.connected()) {
      currentMillis = millis();

      /* Send payload structure */
      if (currentMillis - startMillis >= period){
        u_int8_t requestCode = 1; 

        /* Parse the data*/
        float temperature = readTemperature();
        int b0 = temperature;
        int b1 = (temperature - b0)*100;

        float humidity = readHumidity();
        int b2 = humidity;
        int b3 = (humidity - b2)*100;

        /* +DEBUG: Test the sensor output values */
        //Serial.println(String(temperature) + " " + String(humidity));

        /* Assemble the data for the payload to be sent.*/
        u_int8_t payload[5] = {requestCode,b0,b1,b2,b3};
        
        /*Send the payload through the buffer*/
        dataCharacteristic.writeValue((byte*) payload, sizeof(payload));
        startMillis = currentMillis;
        Serial.println("Data sent!");
      }
     
      /* Receive payload structure */
      if (dataCharacteristic.written()) {
        
        char* receivedData = ( char* )dataCharacteristic.value();
        Serial.println(receivedData);

        if (receivedData[0] == 'T' && receivedData[1] == '0'){
          Serial.println("Period: Set to 1s");
          period = 1000;
        } else if (receivedData[0] == 'T' && receivedData[1] == '1'){
          Serial.println("Period: Set to 10s");
          period = 10000;
        } else if (receivedData[0] == 'T' && receivedData[1] == '2'){
          Serial.println("Period: Set to 30s");
          period = 30000;
        } else if (receivedData[0] == 'T' && receivedData[1] == '3'){
          Serial.println("Period: Set to 1m");
          period = 60000;
        } else if (receivedData[0] == 'T' && receivedData[1] == '4'){
          Serial.println("Period: Set to 5m");
          period = 60000*5;
        } 

      }

    }
  Serial.println("* Disconnected to central device!");
  }
}

void loop() {
  bleController();
}











  

