# WeatherStation

This project was built using an Arduino Uno, it's used to get the readings of the Weather Shield
and transmit the data over to a server using a Wifi Shield.

The server used in this project was an Azure Function, where there was a trigger configured
so the Arduino could make a HTTP POST over to the server.

### USED EQUIPMENT:

- Arduino Uno R3 (A000066)
- Weather Shield (SparkFun DEV-12081)
- Wifi Shield (A000058)
- Battery or Power source for the Arduino


### LIBRARIES:

- SPI.h
- WiFi.h
- Wire.h
- SparkFunMPL3115A2.h
- SparkFunHTU21D.h
- ArduinoJson.h
