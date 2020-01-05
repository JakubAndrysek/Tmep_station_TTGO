/*/////////////////////////////////////////////////////////////////////////////////////////////////
//  Name:           Station_ds18b20_UI
//  Description:    This is an internet weather station built in TTGO T-Display-ESP32.
                    The station displays the time, indoor and outdoor temperature + sends 
                    the actual temperature on the TMEP.CZ server.
//  Author:         Kuba Andrysek
//  Website:        https://kubaandrysek.cz/
////////////////////////////////////////////////////////////////////////////////////////////////*/

#include <TFT_eSPI.h> 
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include <Button2.h>
#include "esp_adc_cal.h"
#include <Arduino.h>
#include <WiFi.h>
#include <OneWire.h>           // OneWire communication library for DS18B20
#include <DallasTemperature.h> // DS18B20 library
#include "time.h"


TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

// Data wire is connected to GPIO15
#define ONE_WIRE_BUS 32
#define MILLIS 1000
// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
DeviceAddress TempIn = { 0x28, 0x1E, 0x77, 0x4A, 0xA, 0x0, 0x0, 0xFB };
DeviceAddress TempOut = { 0x28, 0xFF, 0x27, 0xC8, 0x43, 0x16, 0x4, 0x36 };
// Set display brightness
const int ledPin = 4;  
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int TFTBrightness = 255; // 0-255


// Define settings
// const char ssid[]     = "---ssid---"; // WiFi SSID
// const char pass[]     = "---password---"; // WiFi password
// const char ssid2[]     = "---ssid2---"; // WiFi SSID
// const char pass2[]     = "---password2---"; // WiFi password
// const char domain[]   = "---domain---";  // domain.tmep.cz
// const char guid[]     = "---guid---"; // mojemereni
#include "credentials.h"

//NTPServer setup 
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
 
bool ones = true;
bool connected  = true;
int color = TFT_WHITE;

void gridLight()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.fillRoundRect(0,0, tft.width(), 30, 8, TFT_BLUE); 
    tft.fillRoundRect(0,40, tft.width(), 95, 8, TFT_BLUE);
    tft.fillRoundRect(0,145, tft.width(), 95, 8, TFT_BLUE); 
    color = TFT_WHITE;

}

void gridDark()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_BLACK, TFT_DARKGREY);
    tft.fillRoundRect(0,0, tft.width(), 30, 8, TFT_DARKGREY); 
    tft.fillRoundRect(0,40, tft.width(), 95, 8, TFT_DARKGREY);
    tft.fillRoundRect(0,145, tft.width(), 95, 8, TFT_DARKGREY);
    color = TFT_BLACK;
}


String dwoDigit(int time)
{
    String new_time;
    if (time < 70)
    {
        if (time<=9)
        {
            new_time = "0" + String(time);
        }
        else
        {
            new_time = String(time);
        }
    }
    else
    {
        new_time = String(".");
    }    
    return new_time;
}

//String time = String(tm.tm_hour) + ":" + String(tm.tm_min) + ":" + String(tm.tm_sec);
//tft.drawCentreString(time, tft.width()/2, 12, 4);

float minF(float _now, float _min)
{
    if(_now < _min)
    {
        return _now;
    }
    return _min;
}

float maxF(float _now, float _max)
{
    if(_now > _max)
    {
        return _now;
    }
    return _max;
}


void setup()
{
    Serial.begin(115200);
    Serial.println("Last update 2020-1-4");
    Serial.println("Start");
    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(ledPin, ledChannel);
    ledcWrite(ledChannel, TFTBrightness);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLUE);  
    tft.setTextColor(TFT_WHITE, TFT_BLUE);

  // Connect to the WiFi
    tft.drawCentreString("Connecting", tft.width()/2, 35, 4);
    
    WiFi.begin(ssid, pass); //Suzand
    tft.drawCentreString(String(ssid), tft.width()/2, 60, 4);
    Serial.printf("Connecting to %s\n", ssid);

    int mil_start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if(millis()-mil_start>20000)
        {
            break;
            connected = false;

        }
        else if(millis()-mil_start>10000)
        {
            WiFi.begin(ssid2, pass2); //Technika
            tft.drawCentreString(String(ssid2), tft.width()/2, 60, 4);
            Serial.printf("Connecting to %s\n", ssid2);
        }
        
    }


    Serial.println();
    Serial.println(F("WiFi connected"));
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
    Serial.println();
 
    sensors.begin();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


    gridLight();
}

void loop()
{
    struct Temp
    {
        float now;
        float last;
        float min;
        float max;
    }tempIn, tempOut;

    struct Time
    {
        int now;
        int last;
    }timeTFT, timeDis;  

    
    struct tm tm;
    getLocalTime(&tm);

    bool run = true;
  
    int poc1 = 0;
    int poc2 = 0;   
    int hourLast = 0;
    int mdayLast = 0;

    timeTFT.last = 990;     
    timeDis.last = millis();    
    
    sensors.requestTemperatures(); // Send the command to get temperatures
    tempIn.last = 0;
    tempOut.last = 0;
    tempIn.min = tempIn.max = sensors.getTempC(TempIn);
    tempOut.min = tempOut.max = sensors.getTempC(TempOut);
    
    

    while(run)
    {
        sensors.requestTemperatures(); // Send the command to get temperatures

        
        timeTFT.now  = millis();     
        timeDis.now = millis(); 

        tempIn.now = sensors.getTempC(TempIn);
        tempOut.now = sensors.getTempC(TempOut); 
        
        getLocalTime(&tm);        
        
        if((timeDis.now-timeDis.last)>1*MILLIS) //time is in seconds (15)
        {  
            //Set theme
            if(tm.tm_hour != hourLast)
            {
                if ((tm.tm_hour >= 21) || (tm.tm_hour <= 7))
                {
                    //Dark
                    TFTBrightness = 30;
                    ledcWrite(ledChannel, TFTBrightness);
                    gridDark();
                    tempIn.last = 0;
                    tempOut.last = 0;
                }
                else
                {
                    //Light
                    TFTBrightness = 255;
                    ledcWrite(ledChannel, TFTBrightness);
                    gridLight();
                    tempIn.last = 0;
                    tempOut.last = 0;
                }
            }
            hourLast = tm.tm_hour;


            //Reset min/max values at 12pm
            if (tm.tm_mday != mdayLast)
            {
                
                tempIn.min = tempIn.now;
                tempIn.max = tempIn.now;                
                tempOut.min = tempOut.now;
                tempOut.max = tempOut.now;

                mdayLast = tm.tm_mday;
            }
            
            //Print time
            String time = dwoDigit(tm.tm_hour) + ":" + dwoDigit(tm.tm_min) + ":" + dwoDigit(tm.tm_sec);
            tft.drawCentreString(time, tft.width()/2, 4, 4);

            //Print IN temperatures
            if (tempIn.last!=tempIn.now)
            {
                //Actual   
                tft.drawString(String(tempIn.now), 5, 50, 6);
                tempIn.last = tempIn.now;

                //Min - left
                tempIn.min = minF(tempIn.now, tempIn.min);                
                tft.drawString((String(tempIn.min)), 5, 115, 4);
                tft.drawLine(35, 92,  35, 110, color);
                tft.drawLine(35, 110, 30, 105, color);
                tft.drawLine(35, 110, 40, 105, color);
                tft.fillRoundRect(0, 135, tft.width(), 10, 0, TFT_BLACK);

                //Max - right
                tempIn.max = maxF(tempIn.now, tempIn.max);
                tft.drawString((String(tempIn.max)), 70, 90, 4);
                tft.drawLine(100, 117, 100, 132, color);
                tft.drawLine(100, 117, 95,  122, color);
                tft.drawLine(100, 117, 105, 122, color);                
            }

            //Print OUT temperatures
            if (tempOut.last!=tempOut.now)
            {
                //Actual
                tft.drawString(String(tempOut.now), 5, 150, 6);
                tempOut.last = tempOut.now;
                
                //Min - left
                tempOut.min = minF(tempOut.now, tempOut.min);                
                tft.drawString((String(tempOut.min)), 5, 215, 4);
                tft.drawLine(35, 192, 35, 210, color);
                tft.drawLine(35, 210, 30, 205, color);
                tft.drawLine(35, 210, 40, 205, color);

                //Max - right
                tempOut.max = maxF(tempOut.now, tempOut.max);
                tft.drawString((String(tempOut.max)), 70, 190, 4);
                tft.drawLine(100, 217, 100, 235, color);
                tft.drawLine(100, 217, 95,  222, color);
                tft.drawLine(100, 217, 105, 222, color);                
            }


            timeDis.last = timeDis.now;
            poc1++;
        }


        if((timeTFT.now-timeTFT.last)>60*MILLIS) //time is in seconds (15)
        {

            // Connect to the HOST and send data via GET method
            WiFiClient client; // Use WiFiClient class to create TCP connections
            
            char host[50];            // Joining two chars is little bit difficult. Make new array, 50 bytes long
            strcpy(host, domain);     // Copy /domain/ in to the /host/
            strcat(host, ".tmep.cz"); // Add ".tmep.cz" at the end of the /host/. /host/ is now "/domain/.tmep.cz"
            
            Serial.print("Connecting to "); Serial.println(host);
            if (!client.connect(host, 80)) {
                // If you didn't get a connection to the server
                Serial.println("Connection failed");
                // Blink 3 times when host connection error
                delay(1000);
                return;
            }
            Serial.println(F("Client connected"));

            // Make an url. We need: /?guid=t
            String url = "/?";
                    url += guid;
                    url += "=";
                    url += tempOut.now;
                    url += "&humV=";
                    url += tempIn.now;
            Serial.print("Requesting URL: "); Serial.println(url);
            
            // Make a HTTP GETrequest.
            client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" + 
                        "Connection: close\r\n\r\n");
            
            
            // Workaroud for timeout
            unsigned long timeout = millis();
            while (client.available() == 0) {
                if (millis() - timeout > 5000) {
                    Serial.println(F(">>> Client Timeout !"));
                    client.stop();
                    delay(1000);
                    return;
                }
            }            
            Serial.println();

            timeTFT.last = timeTFT.now;        
            poc2++;
            
        } 
    }

}
