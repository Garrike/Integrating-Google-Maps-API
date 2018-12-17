#include <Arduino.h>
#include <GoogleMapsDirectionsApi.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>
#include <String.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

//Credentials for Google GeoLocation API...
const char* Host = "www.googleapis.com";
String thisPage = "/geolocation/v1/geolocate?key=";
String key = "";

int status = WL_IDLE_STATUS;
String jsonString = "{\n";

double latitude    = 0.0;
double longitude   = 0.0;

//------- Replace the following! ------
char ssid[] = "";           //your network SSID (name)
char password[] = "";       // your network key
#define API_KEY ""  // your google apps API Token

// Initialize Telegram BOT
#define BOTtoken ""  // your Bot Token (Get from Botfather)

WiFiClientSecure client;
GoogleMapsDirectionsApi api(API_KEY, client);

UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

//Free Google Maps Api only allows for 2500 "elements" a day, so carful you dont go over
unsigned long api_mtbs = 60000; //mean time between api requests
unsigned long api_due_time = 0;
bool firstTime = true;
int aux = 1;
String ID = "0";

//Inputs
String origin = "";                 //Exemple: "53.338877, -6.262093" or Dublin, Ireland
String destination = "";            //Exemple: "51.897078, -8.475921" or Cork, Ireland
String waypoints = "";      //You need to include the via: before your waypoint

//Optional
DirectionsInputOptions inputOptions;

void geolocation();
void checkGoogleMaps();

void action(int value)
{
  if (value == 0) Serial.print("turn-sharp-left ");
  else if (value == 1) Serial.print("uturn-right ");
  else if (value == 2) Serial.print("turn-slight-right ");
  else if (value == 3) Serial.print("merge ");
  else if (value == 4) Serial.print("roundabout-left ");
  else if (value == 5) Serial.print("roundabout-right ");
  else if (value == 6) Serial.print("uturn-left ");
  else if (value == 7) Serial.print("turn-slight-left ");
  else if (value == 8) Serial.print("turn-left");
  else if (value == 9) Serial.print("ramp-right ");
  else if (value == 10) Serial.print("turn-right"); 
  else if (value == 11) Serial.print("fork-right ");
  else if (value == 12) Serial.print("straight ");
  else if (value == 13) Serial.print("fork-left ");
  else if (value == 14) Serial.print("ferry-train ");
  else if (value == 15) Serial.print("turn-sharp-right ");
  else if (value == 16) Serial.print("ramp-left ");
  else if (value == 17) Serial.print("ferry ");
  else if (value == 18) Serial.print("Head northeast");
  else if (value == 19) Serial.print("Head northwest");
  else if (value == 20) Serial.print("Head north");
  else if (value == 21) Serial.print("Head southeast");
  else if (value == 22) Serial.print("Head southwest");
  else if (value == 23) Serial.print("Head south");
  else if (value == 24) Serial.print("Head east");
  else if (value == 25) Serial.print("Head west");
}

void handleNewMessages(int numNewMessages, int option) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (bot.messages[i].longitude != 0 || bot.messages[i].latitude != 0) {
      Serial.print("Lat: ");
      Serial.println(String(bot.messages[i].latitude, 6));
      String lat_telegram = String(bot.messages[i].latitude, 6);
      Serial.print("Long: ");
      Serial.println(String(bot.messages[i].longitude, 6));
      String lon_telegram = String(bot.messages[i].longitude, 6);

      String message = "Long: " + String(bot.messages[i].longitude, 6) + "\n";
      message += "Lat: " + String(bot.messages[i].latitude, 6) + "\n";
      bot.sendMessage(chat_id, message, "Markdown");

      if (option == 1)
      {
        ID = chat_id;
        geolocation();
        String a = String(latitude, 6);
        String b = String(longitude, 6);
        origin = String(a + ", " + b);
        Serial.println(origin);
        destination = String(lat_telegram + ", " + lon_telegram);
        Serial.println(destination);
        inputOptions.waypoints = "";
        checkGoogleMaps();
      } else if (option == 2) {
        ID = chat_id;
        if(aux == 1) {
          origin = String(lat_telegram + ", " + lon_telegram);
          aux = 2;
        } else {
          destination = String(lat_telegram + ", " + lon_telegram);
          inputOptions.waypoints = "";
          checkGoogleMaps();
          aux = 1;
        }
      }
      Serial.println("");
      
    } else if (text == "/start") {
      String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
      welcome += "Share a location or a live location and the bot will respond with the co-ords\n";

      bot.sendMessage(chat_id, welcome, "Markdown");
    }
    ID = "0";
  }
}

void geolocation()
{
    DynamicJsonBuffer jsonBuffer;
    int n = WiFi.scanNetworks();
  
    // now build the jsonString...
    jsonString="{\n";
    jsonString +="\"homeMobileCountryCode\": 234,\n";  // this is a real UK MCC
    jsonString +="\"homeMobileNetworkCode\": 27,\n";   // and a real UK MNC
    jsonString +="\"radioType\": \"gsm\",\n";          // for gsm
    jsonString +="\"carrier\": \"Vodafone\",\n";       // associated with Vodafone 
    jsonString +="\"wifiAccessPoints\": [\n";
    for (int j = 0; j < n; ++j)
    {
      jsonString +="{\n";
      jsonString +="\"macAddress\" : \"";    
      jsonString +=(WiFi.BSSIDstr(j));
      jsonString +="\",\n";
      jsonString +="\"signalStrength\": ";     
      jsonString +=WiFi.RSSI(j);
      jsonString +="\n";
      if(j<n-1)
      {
      jsonString +="},\n";
      }
      else
      {
      jsonString +="}\n";  
      }
    }
    jsonString +=("]\n");
    jsonString +=("}\n"); 
  //--------------------------------------------------------------------  
  
    //Connect to the client and make the api call  
    if (client.connect(Host, 443)) {
      Serial.println("Google Geolocation");    
      client.println("POST " + thisPage + key + " HTTP/1.1");    
      client.println("Host: "+ (String)Host);
      client.println("Connection: close");
      client.println("Content-Type: application/json");
      client.println("User-Agent: Arduino/1.0");
      client.print("Content-Length: ");
      client.println(jsonString.length());    
      client.println();
      client.print(jsonString); 
      yield(); 
      delay(500);
    }
  
    //Read and parse all the lines of the reply from server          
    while (client.available()) {
      String line = client.readStringUntil('\r');
      JsonObject& root = jsonBuffer.parseObject(line);
      if(root.success()){
      latitude    = root["location"]["lat"];
      longitude   = root["location"]["lng"];
      }
      yield();
    }
    client.stop();
  
    Serial.print("Latitude = ");
    Serial.println(latitude,6);
    Serial.print("Longitude = ");
    Serial.println(longitude,6);
}

void setup() {

  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  //These are all optional (although departureTime needed for traffic)
  inputOptions.departureTime = "now"; //can also be a future timestamp
  inputOptions.trafficModel = "best_guess"; //Defaults to this anyways
  inputOptions.avoid = "ferries";
  inputOptions.units = "metric";
}

void checkGoogleMaps() {

  Serial.println("");
  Serial.println("Getting traffic for " + origin + " to " + destination);
  DirectionsResponse response = api.directionsApi(origin, destination, inputOptions);
  Serial.println("");
  Serial.println("Response:");
  Serial.println("");
  
  Serial.print("Trafic from ");
  Serial.print(response.start_address);
  Serial.print(" to ");
  Serial.println(response.end_address);
  
  Serial.print("Duration in Traffic text: ");
  Serial.println(response.durationTraffic_text);
  
  Serial.print("Normal duration text: ");
  Serial.println(response.duration_text);
  
  Serial.print("Distance text: ");
  Serial.println(response.distance_text);
  
  Serial.println("");
  EEPROM.begin(2000);
  int i = 2, j = 3, cont = 0;
  int aux = EEPROM.read(1), soma = 0;
  Serial.println("Maneuver: ");

  while(cont < response.maneuver)
  {
    action(EEPROM.read(i));
    if(EEPROM.read(j) == 0)
    {
      while(EEPROM.read(aux) != 0)
      {
        soma = soma + EEPROM.read(aux);
        aux++;
      }
      aux++;
      EEPROM.write(1, aux);
      EEPROM.commit();
      Serial.print(soma);
      if (ID != "0"){
        String message = String(soma);
        message += " metros\n";
        bot.sendMessage(ID, message, "Markdown");
      }
      soma = 0;
    }
    else {
      Serial.print(EEPROM.read(j));
      if (ID != "0"){
        String message = String(EEPROM.read(j));
        message += " metros\n";
        bot.sendMessage(ID, message, "Markdown");
      }
    }

    Serial.println(" m");
    i = i + 2;
    j = j + 2;
    cont++;
  }
  EEPROM.write(1, 100);
  EEPROM.commit();
  EEPROM.end();
}

void loop() {
  if(Serial.available() > 0)
  {
    int val = Serial.parseInt();
    if(val == 0) // Predefined origin and destination
    {
      origin = "";   // Set origin
      destination = ""; // Set destination
      inputOptions.waypoints = ""; // Set waypoint
      checkGoogleMaps();
    }
    if(val == 1) // Origin by Google Geolocation and Destination by Telegram
    {
      int numNewMessages = 0;
      while (numNewMessages == 0) {
        delay(1000);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      
      while (numNewMessages) {
        Serial.println("got response");
        handleNewMessages(numNewMessages, 1);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
    }
    if(val == 2) // Origin and Destination chosen by Telegram
    {
      int numNewMessages = 0;
      int x = 0;
      while (x < 2) {
        delay(1000);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);

        if (numNewMessages == 1)
        {
          Serial.println("got response");
          handleNewMessages(numNewMessages, 2);
          numNewMessages = bot.getUpdates(bot.last_message_received + 1);
          x++;
        }      
        x = x + numNewMessages;
      }
    }
  }
}
