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
#include "credentials.h"

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

// Data wire is connected to GPIO15
#define ONE_WIRE_BUS 32
#define MILLIS 1000
// Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1 = { 0x28, 0x1E, 0x77, 0x4A, 0xA, 0x0, 0x0, 0xFB };
DeviceAddress sensor2 = { 0x28, 0xFF, 0x27, 0xC8, 0x43, 0x16, 0x4, 0x36 };
// Set display brightness
const int ledPin = 4;  
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int TFTBrightness = 255; // 0-255

// Define settings
const char ssid[]     = "_SSID_"; // WiFi SSID
const char pass[]     = "_PWD_"; // WiFi password
const char ssid2[]     = "_SSID2_"; // WiFi SSID
const char pass2[]     = "_PWD2_"; // WiFi password
const char domain[]   = "_DOMAIN_";  // domain.tmep.cz
const char guid[]     = "_GUID_"; // mojemereni

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const byte sleepInMinutes = 1; // How often send data to the server. In minutes 
 
bool ones = true;
bool connected  = true;

void gridLight()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.fillRoundRect(0,0, tft.width(), 30, 8, TFT_BLUE); 
    tft.fillRoundRect(0,40, tft.width(), 95, 8, TFT_BLUE);
    tft.fillRoundRect(0,145, tft.width(), 95, 8, TFT_BLUE); 
    delay(5000) ;

}

void gridDark()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.fillRoundRect(0,0, tft.width(), 30, 8, TFT_BLACK); 
    tft.fillRoundRect(0,50, tft.width(), 150, 8, TFT_BLACK);
    tft.fillRoundRect(0,180, tft.width(), tft.height(), 8, TFT_BLACK);
   
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

void setup()
{
    Serial.begin(115200);
    Serial.println("Last update 2019-12-2");
    Serial.println("Start");
    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(ledPin, ledChannel);
    ledcWrite(ledChannel, TFTBrightness);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLUE);  
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    gridLight();
}

void loop()
{
    struct Temp
    {
        float now;
        float last;
    }tempIn, tempOut;

    struct Time
    {
        int now;
        int last;
    }timeTFT, timeDis;  

    enum Theme
    {
        None,
        Light,
        Dark
    };

    Theme lastTheme = None, nowTheme;
    
    struct tm tm;

    bool run = true;
  
    int poc1 = 0;
    int poc2 = 0;   
    int hour = 5;
    int hourLast = 4;

    timeTFT.last = 990;     
    timeDis.last = millis();       

    tft.drawCentreString("Ahoj", tft.width()/2, 50, 4);

    while(run)
    {
        timeTFT.now  = millis();     
        timeDis.now = millis();  
        
        if((timeDis.now-timeDis.last)>1000)
        {  
            if(hour != hourLast)
            {
                if ((hour > 21) || (hour < 7))
                {
                    TFTBrightness = 70;
                    ledcWrite(ledChannel, TFTBrightness);
                    gridDark();
                }
                else
                {
                    TFTBrightness = 255;
                    ledcWrite(ledChannel, TFTBrightness);
                    gridLight();
                }

            }
            hourLast = hour;
            
            String time = dwoDigit(2) + ":" + dwoDigit(15) + ":" + dwoDigit(millis());
            tft.drawCentreString(time, tft.width()/2, 4, 4);


            timeDis.last = timeDis.now;
            poc1++;
        }


        if((timeTFT.now-timeTFT.last)>15*MILLIS)
        {
            timeTFT.last = timeTFT.now;        
            poc2++;
            hour = 23;
            hourLast = 22;
            
        } 
    }

}
