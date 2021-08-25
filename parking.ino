#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* serverName = "http://13.232.174.224";

const char* ssid = "SHRIHARIHOME";
const char* password = "9952666934";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

LiquidCrystal_I2C lcd[3] = {
  LiquidCrystal_I2C(0x27,16,2),
  LiquidCrystal_I2C(0x25,16,2),
  LiquidCrystal_I2C(0x26,16,2)
};

String vehicle_number[2];
String slot[2];
int directions_input[2][3];
float hours = 3;
int cost = 50;
int flag = 0;
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
//  count = count - 1;
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
  wifi_connection();
  lcd[0].home();
  lcd[1].home();
  lcd[2].home();
  lcd[0].clear();
  lcd[1].clear();
  lcd[2].clear();
}

void http_req(String num, int counter)
{
  String httprequestdata = "vehicleNum=" + num;
  Serial.println(httprequestdata);
  
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

      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");
      Serial.println("CALLING SERVER");
      int http_res = http.POST(JSONmessageBuffer);

      Serial.println(http_res);
      if (http_res == 200){
        
        const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(1) + 60;
        DynamicJsonBuffer jsonBuffer(capacity);
        Serial.println("Server Response: ");
        
        Serial.print("Server Status Code: ");
        Serial.println("Received");
        String payload = http.getString();
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (!root.success()) {
          Serial.println("Parsing failed!");
          return;
        } 
        else 
        {
          Serial.println(root["message"].as<char*>());
          Serial.println(root["lot"].as<char*>());
          Serial.println(root["directions"].as<char*>());
          slot[counter] = root["lot"].as<char*>();
          if (root["message"].as<char*>() == "Parking Lot found")
          {
            directions_input[counter][0] = root["directions"][0].as<int>();
            directions_input[counter][1] = root["directions"][1].as<int>();
            directions_input[counter][2] = root["directions"][2].as<int>();
            flag = 0;
          }
          else
            flag = 1;
        }
      }
      else {
        Serial.println("wrong request");
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

int counter = 0;
void loop() {
  delay(500);
  Serial.println("Enter vehicle number:");
  delay(10000);
  if (Serial.available() > 0)  // is a character available?
  {   
    rx_str = Serial.readString();       // get the character
  }
  Serial.println(rx_str);
  delay(1000);
//  while(counter <= 2)
//  {
    if (counter == 0 && rx_str != vehicle_number[1])
    {
      vehicle_number[counter] = rx_str; 
      http_req(rx_str, counter);
      if (flag == 0)
      {
        directions(counter, 0);
        delay(5000);
      }
      counter = 1;
    }
    else if(counter == 1 && rx_str != vehicle_number[0])
    {
      vehicle_number[counter] = rx_str;
      http_req(rx_str, counter);
      if (flag == 0)
      {
        directions(counter, 1);
        delay(5000);
      }
      counter = 0;
    }
//  }
  exit_park(0); 
}
