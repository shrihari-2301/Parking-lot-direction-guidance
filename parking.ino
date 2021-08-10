#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* serverName = "http://13.232.174.224";

const char* ssid = "Porsche@Autobahn";
const char* password = "$anjay2001!";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;


LiquidCrystal_I2C lcd[3] = {
  LiquidCrystal_I2C(0x27,16,2),
  LiquidCrystal_I2C(0x25,16,2),
  LiquidCrystal_I2C(0x26,16,2)
};

char vehicle_number[3][15] = {"TN37BX1816", "TN38CT8055","TN37HU9090"};
char slot[3][5] = {"C1","D2","D4"};
int directions_input[3][3] ={{0,2,3},{0,1,3},{0,1,3}};
float hours = 3;
int cost = 50;
String rx_str = "";


byte STRAIGHT[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte RIGHT_1[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};

byte RIGHT_2[] = {
  B11000,
  B01100,
  B00110,
  B11111,
  B11111,
  B00110,
  B01100,
  B11000
};

byte LEFT_1[] = {
  B00011,
  B00110,
  B01100,
  B11111,
  B11111,
  B01100,
  B00110,
  B00011
};

byte LEFT_2[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};

void straight(int lc, int j, int count)
{
  if(j == 0)
    lcd[lc].init();
    
  lcd[lc].backlight();
  lcd[lc].setCursor(0, j);
  lcd[lc].print(vehicle_number[count]);  
  

  lcd[lc].setCursor(11,j);
  lcd[lc].print(slot[count]);

  lcd[lc].createChar(0, STRAIGHT);
  lcd[lc].setCursor(14, j);
  lcd[lc].write(0);

  lcd[lc].createChar(1 , STRAIGHT);
  lcd[lc].setCursor(15, j);
  lcd[lc].write(1);
    
}

void right(int lc, int j, int count)
{
  if(j == 0)
    lcd[lc].init();
  
  lcd[lc].backlight();
  
  lcd[lc].setCursor(0, j);
  lcd[lc].print(vehicle_number[count]);
  lcd[lc].setCursor(11,j);
  lcd[lc].print(slot[count]);

  lcd[lc].createChar(2, RIGHT_1);
  lcd[lc].setCursor(14, j);
  lcd[lc].write(2);

  lcd[lc].createChar(3, RIGHT_2);
  lcd[lc].setCursor(15, j);
  lcd[lc].write(3);

}

void left(int lc, int j, int count)
{
  if(j == 0)
    lcd[lc].init();
    
  lcd[lc].backlight();
  lcd[lc].setCursor(0, j);
  lcd[lc].print(vehicle_number[count]);

  lcd[lc].setCursor(11,j);
  lcd[lc].print(slot[count]);

  lcd[lc].createChar(4, LEFT_1);
  lcd[lc].setCursor(14, j);
  lcd[lc].write(4);

  lcd[lc].createChar(5, LEFT_2);
  lcd[lc].setCursor(15, j);
  lcd[lc].write(5);

}

void exit_park(int j)
{
  lcd[2].init();
  lcd[2].backlight();
//  lcd[2].print(vehicle_number[j]);
  lcd[2].print(rx_str);
  lcd[2].setCursor(10, 0);
  lcd[2].print("-");
  lcd[2].setCursor(11, 0);
  lcd[2].print("Rs.");
  lcd[2].setCursor(14, 0);
  lcd[2].print(cost);
  lcd[2].setCursor(0, 1);
  lcd[2].print("Hours - ");
  lcd[2].setCursor(8, 1);
  lcd[2].print(hours);
}
  
void directions(int count, int j)        //prints the directions for a car
{
  count -= 1;
  for(int  i = 0; i<3 ;++i)
  {  
    if (directions_input[count][i] != 3)
    {
      if (directions_input[count][i] == 0)
          straight(i,j, count);
      if (directions_input[count][i] == 1)
          right(i, j, count);
      if (directions_input[count][i] == 2)
          left(i, j, count);
    }
  }
}

/*
  0 - straight
  1 - right
  2 - left
  3 - None 
*/
void wifi_connection()
{
  WiFi.begin(ssid,password);
  Serial.print("WiFi connecting ");
  while (WiFi.status() !=WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  wifi_connection();  // connecting to the wifi
}

//StaticJsonDocument<256> jsonBuffer;
//const String httpdata[3] ; 

void http_req(String num)
{
  String httprequestdata = "vehicleNum=" + num;
  Serial.println(httprequestdata);
//  strcat(httprequestdata, num);
//  char json[] = "{\"message\":,\"Lot\":,\"directions\":}";
  
  // Send an HTTP POST request depending on timerDelay
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status() == WL_CONNECTED){
      WiFiClient client;
      
      HTTPClient http;

      StaticJsonBuffer<300> JSONbuffer;

      JsonObject& JSONencoder = JSONbuffer.createObject();

      JSONencoder["vehicleNum"] = num;

      char JSONmessageBuffer[300];

      JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

      Serial.println(JSONmessageBuffer);
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");
//      JsonObject root = deserializeJson(jsonBuffer, http.POST(num));
      
//      DeserializationError error = deserializeJson(jsonBuffer, http.POST(httprequestdata));
        int http_res = http.POST(JSONmessageBuffer);
//      char* dir = jsonBuffer["directions"];
//      char* lt = jsonBuffer["lot"];
//      char* msg = jsonBuffer["message"];
//       String httpdata = "{}";
        Serial.print("Server Response: ");
        Serial.println(http.getString());
        Serial.print("Server Status Code: ");
//        Serial.println(httpdata[2]);
      Serial.println(http_res);
      if (http_res == 200){
        Serial.println("Received");
//        if (strcmp(httpdata[0], "0") == 0) {
//        Serial.print("Slot: ");
//        Serial.println(httpdata[1]);
//        Serial.print("Directions: ");
//        Serial.println(httpdata[2]);
//        }
      } 
      else {
//        Serial.println("Parking lot not found");
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

void loop() {
  delay(500);
  Serial.println("Enter vehicle number:");
  delay(10000);
  if (Serial.available() > 0)  // is a character available?
  {   
    rx_str = Serial.readString();       // get the character
  }
  Serial.println(rx_str);
  delay(5000);
 
  http_req(rx_str);
  
  int counter = 1;
  while(counter <= 3)
  {
    if (counter%2 != 0)
    {
      directions((counter), 0);
      delay(5000);
      counter += 1;
    }
    else
    {
      directions((counter), 1);
      delay(5000);
      counter += 1;
    }
  }
  exit_park(0); 
}
