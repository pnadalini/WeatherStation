/*====================================REGION LIBRARIES===================================*/
#include <SPI.h> 
#include <Wire.h>              // I2C needed for sensors
#include "SparkFunMPL3115A2.h" // Pressure sensor - Search "SparkFun MPL3115" and install from Library Manager
#include "SparkFunHTU21D.h"    // Humidity sensor - Search "SparkFun HTU21D" and install from Library Manager
#include <WiFi.h> 
#include <ArduinoJson.h>       // Json object - Search "ArduinoJson" and install the newest version that's not in beta

/*====================================REGION VARIABLES===================================*/
MPL3115A2 pressureSensor;  // Create an instance of the pressure sensor
HTU21D humiditySensor;     // Create an instance of the humidity sensor
WiFiServer server(80);
WiFiClient client;

/*==================================== HARDWARE PINS ====================================*/
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;

/*====================================GLOBAL VARIABLES===================================*/
char ssid[] = "<ssid>"; // Your network SSID (name) 
char pass[] = "<password>";         // Your network password
int status = WL_IDLE_STATUS;

/*====================================ARDUINO METHODS===================================*/

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing components");

  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);

  configureSensors();

  Serial.println("Weather Shield online!");

  // start serial port for debugging purposes
  //Serial.begin(9600);

  // Attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    // Wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // You're connected now, so print out the status:
  printWifiStatus();
}


void loop() {
  // Post the Weather Shield readings to the server
  post();
  waitForResponse();
  readResponse();
  // Give the web browser time to receive the data and wait time so the next weather reading begins
  delay(10000);
  // Close the connection:
  client.stop();
  Serial.println("client disonnected");
  Serial.println("------------------------------------------------");
}

/*====================================CREATED METHODS===================================*/

void configureSensors(){
  // Configure the pressure sensor
  pressureSensor.begin(); // Get sensor online
  pressureSensor.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  pressureSensor.setOversampleRate(7); // Set Oversample to the recommended 128
  pressureSensor.enableEventFlags(); // Enable all three pressure and temp event flags

  // Configure the humidity sensor
  humiditySensor.begin();
}

void post() {
  Serial.println("Connecting to Azure...");

  if (client.connect("<function_name>.azurewebsites.net", 80)) {
    Serial.println("Conected!, preparing post...");
    client.println("POST /api/<trigger_name> HTTP/1.1");
    client.println("Host: <function_name>.azurewebsites.net");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    getWeatherReadings();
  } else {
    Serial.println("connection failed");
  }
}

void getWeatherReadings() {
  // Check Humidity Sensor
  float humidity = humiditySensor.readHumidity();

  if (humidity == 998) {
    // Humidty sensor failed to respond
    Serial.println("I2C communication to sensors is not working. Check solder connections.");

    //Try re-initializing the I2C comm and the sensors
    configureSensors();
  } else {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &postMessage = jsonBuffer.createObject();

    // Check all reading (Temperature, Pressure, Light)
    float tempC = humiditySensor.readTemperature();
    float pressure = pressureSensor.readPressure();
    float tempf = pressureSensor.readTempF();
    float lightLvl = getLightLevel();

    // Put all readings on the Json Object
    postMessage["Humidity"] = humidity;
    postMessage["TempC"] = tempC;
    postMessage["Pressure"] = pressure;
    postMessage["TempF"] = tempf;
    postMessage["LightLvl"] = lightLvl;

    // Write all the readings on the client
    client.println(postMessage.measureLength());
    client.println();
    Serial.println("Json sent to the server:");
    postMessage.printTo(Serial);
    Serial.println();
    postMessage.printTo(client);
  }
}

void waitForResponse() {
  Serial.println("Waiting for response from server..");
  while (!client.available()) {
      if (!client.connected()) {
          return;
      }
  }
}

void readResponse() {
  Serial.println("*****************RESPONSE******************");
  while (client.available()) {
      char c = client.read();
      Serial.print(c);
  }
}

// Returns the voltage of the light sensor based on the 3.3V rail
// This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
float getLightLevel() {
  float operatingVoltage = analogRead(REFERENCE_3V3);
  float lightSensor = analogRead(LIGHT);
  
  operatingVoltage = 3.3 / operatingVoltage; // The reference voltage is 3.3V
  lightSensor = operatingVoltage * lightSensor;

  return (lightSensor);
}

void printWifiStatus() {
  // Print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
