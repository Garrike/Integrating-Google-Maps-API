# Integrating-Google-Maps-API

The goal of this project is to be able to integrate all available functions of the Google Maps API into a microcontroller,
 in this case the ESP8266 - NODEMCU, that already has a built-in Wi-Fi module.

First you need to activate a key on the Google Clouds and Telegram platform. Below are the steps to this:

(IMAGES: Slides 6, 7, 8, 10 and 25)

![alt text](https://help.github.com/assets/images/help/repository/readme-links.png)

Next, install 4 required libraries for the code:

https://github.com/witnessmenow/arduino-google-maps-api

https://github.com/squix78/json-streaming-parser

https://github.com/bblanchon/ArduinoJson

https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot

Before you run the code, you need to make another change. For my application, I had to save the data permanently, so 
that I applied to the Google Maps API only once and then could also replace the code made for geolocation by a GPS 
module.

So I added a few lines in the Google Maps library to be able to receive directions and distances only once and store 
them in the ESP8266 EEPROM memory. So, replace the "GoogleMapsDirectionsApi.cpp" and "GoogleMapsDirectionsApi.h" 
files from the arduino-google-maps-api / src / folder in the "arduino-google-maps-api" library with the two files 
with the same name found here in this repository . If you do not want to, you can also just adapt the instructions 
for the meneuvers to html_instructions.

As input method, if you enter 0 on the serial port, the origin and destination values ​​used will be those already 
predefined in the code. If the entry is 1, it will use its geolocation as the origin variable, and the destination 
will be the location sent by the Telegram. Finally, by typing 3 in the serial, you can send two locations through the 
Telegram the first being worked as the origin and the second as the destination. 1