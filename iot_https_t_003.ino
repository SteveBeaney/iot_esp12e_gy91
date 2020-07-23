#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <Wire.h>

#include "MPU9250.h"
#include <Adafruit_NeoPixel.h>
#include <BMP280_DEV.h>                           // Include the BMP280_DEV.h library


#define PIN        14 
#define NUMPIXELS   4 
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);


const char* apSsid = "ESPAdmin";
const char* apPassword = "12345678";
String ssid = "b1";
String password = "ed174b4c5e";
int repeat = 5;
unsigned long tick = millis()-(repeat*1000);
String apikey = "No key";
String host = "sbeaney.com";
String base = "test";
String device = "No device name";
int rgbRed = 40;
int rgbGreen = 0;
int rgbBlue = 0;
String settings = apikey+"\t"+device+"\t"+ssid+"\t"+password+"\t"+repeat+"\t"+host+"\t"+base;
char ID[40];
char clientIp[200];
char apIp[200];

BMP280_DEV bmp280;

MPU9250 IMU(Wire,0x68);
int status;

ESP8266WebServer server(80); //Server on port 80
IPAddress dns(8, 8, 8, 8);

void handleRoot() {
    String b = (String)"<!DOCTYPE html>"+
               "<html lang=\"en\">"+
               "<head>"+
                   "<meta charset=\"UTF-8\">"+
                   "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"+
                   "<title>Admin settings</title>"+
               "</head>"+
               "<body>"+
                   "<h2>IOT "+ID+"</h2>"+
                   "<form action=\"/data\">"+
                       "<table cellpadding=3 cellspacing=5>"+
                           "<tr><th>IoT settings:</th><td>&nbsp;</td><td>&nbsp;</td></tr>"+
                           
                           "<tr><td>&nbsp;</td><td><label for=\"host\">Host:</label></td>"+
                           "<td><input type=\"text\" id=\"host\" name=\"host\" value=\""+host+"\"></td></tr>"+

                           "<tr><td>&nbsp;</td><td><label for=\"base\">Base:</label></td>"+
                           "<td><input type=\"text\" id=\"base\" name=\"base\" value=\""+base+"\"></td></tr>"+

                           "<tr><td>&nbsp;</td><td><label for=\"apikey\">Api Key:</label></td>"+
                           "<td><input type=\"text\" id=\"apikey\" name=\"apikey\" value=\""+apikey+"\"></td></tr>"+

                           "<tr><td>&nbsp;</td><td><label for=\"device\">Device:</label></td>"+
                           "<td><input type=\"text\" id=\"device\" name=\"device\" value=\""+device+"\"></td></tr>"+
                       
                           "<tr><td>&nbsp;</td><td><label for=\"repeat\">Repeat:</label></td>"+
                           "<td><input type=\"text\" id=\"repeat\" name=\"repeat\" value=\""+repeat+"\"><br></td></tr>"+
                       
                           "<tr><th>Wifi connection:</th><td>&nbsp;</td><td>&nbsp;</td></tr>"+
                       
                           "<tr><td>&nbsp;</td><td><label for=\"ssid\">Client ip:</label></td>"+
                           "<td><input readonly type=\"text\" id=\"ssid\" name=\"ssid\" value=\""+WiFi.localIP().toString()+"\"></td></tr>"+
                       
                           "<tr><td>&nbsp;</td><td><label for=\"ssid\">Access Point ip:</label></td>"+
                           "<td><input readonly type=\"text\" id=\"ssid\" name=\"ssid\" value=\""+WiFi.softAPIP().toString()+"\"></td></tr>"+
                       
                           "<tr><td>&nbsp;</td><td><label for=\"ssid\">SSID:</label></td>"+
                           "<td><input type=\"text\" id=\"ssid\" name=\"ssid\" value=\""+ssid+"\"></td></tr>"+
                       
                           "<tr><td>&nbsp;</td><td><label for=\"pwd\">Password:</label></td>"+
                           "<td><input type=\"text\" id=\"pwd\" name=\"pwd\" value=\""+password+"\"></td></tr>"+
           
                           "<tr><td><input type=\"submit\" value=\"Submit\"></td><td>&nbsp;</td><td>&nbsp;</td></tr>"+
                       "</table>"
                   "</form>"+
                   "<hr>"+
                   
                   "<form action=\"/data\">"+
                       "<table>"
                           "<tr><th>RGB light:</th><td>&nbsp;</td><td>&nbsp;</td></tr>"+
                       
                           "<tr><td>&nbsp;</td><td><label for=\"rgbRed\">Red:</label></td>"+
                           "<td><input type=\"text\" id=\"rgbRed\" name=\"rgbRed\" value=\""+rgbRed+"\"></td></tr>"+
                       
                           "<tr><td>&nbsp;</td><td><label for=\"rgbGreen\">Green:</label></td>"+
                           "<td><input type=\"text\" id=\"rgbGreen\" name=\"rgbGreen\" value=\""+rgbGreen+"\"></td></tr>"+
                       
                           "<tr><td>&nbsp;</td><td><label for=\"rgbBlue\">Blue:</label></td>"+
                           "<td><input type=\"text\" id=\"rgbBlue\" name=\"rgbBlue\" value=\""+rgbBlue+"\"></td></tr>"+
                       
                           "<tr><td><input type=\"submit\" value=\"Submit\"></td><td>&nbsp;</td><td>&nbsp;</td></tr>"+
                       "</table>"
                   "</form>"+               
                   "</body>"+
                   "</html>";
    server.send(200, "text/html", b.c_str() );
}


void handleData() {
    int resetWifi = 0;
    int save = 0;
    for (int i = 0; i < server.args(); i++) {
        if( server.argName(i).equals("apikey") ){
            apikey = server.arg(i);
            save=1;
        }
        if( server.argName(i).equals("host") ){
            host = server.arg(i);
            save=1;
        }
        if( server.argName(i).equals("base") ){
            base = server.arg(i);
            save=1;
        }
        if( server.argName(i).equals("device") ){
            device = server.arg(i);
            save=1;
        }
        if( server.argName(i).equals("repeat") ){
            repeat = atoi(server.arg(i).c_str());
            save=1;
        }
        if( server.argName(i).equals("ssid") ){
            ssid = server.arg(i);
            resetWifi = 1;
        }
        if( server.argName(i).equals("pwd") ){
            password = server.arg(i);
            resetWifi = 1;
        }
        if( server.argName(i).equals("rgbRed") ){
            rgbRed = atoi(server.arg(i).c_str());
        }        
        if( server.argName(i).equals("rgbGreen") ){
            rgbGreen = atoi(server.arg(i).c_str());
        }        
        if( server.argName(i).equals("rgbBlue") ){
            rgbBlue = atoi(server.arg(i).c_str());
        }
    } 
    if( resetWifi || save ) {
        settings = apikey + "\t" + device + "\t" + ssid + "\t" + password + "\t" + repeat + "\t" + host + "\t" + base;
      EEPROM.write(0,4);
      writeStringToEEPROM(1,settings);
      EEPROM.commit();
    }
    if( resetWifi ){
        Serial.println("reset wifi due to setting change");
            WiFi.begin(ssid.c_str(),password.c_str());
            while (WiFi.status() != WL_CONNECTED) {
                Serial.print(".");
                delay(300);
           }
    }
    pixels.clear(); 
    for(int i=0; i<NUMPIXELS; i++) { 
        pixels.setPixelColor(i, pixels.Color(rgbRed, rgbGreen, rgbBlue));
    }
    pixels.show();   
    handleRoot();
}

void connectWiFi() {
    Serial.println("Starting wifi");
    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    sprintf(ID,"BIAB:%02x:%02x:%02x\n",  macAddr[3], macAddr[4], macAddr[5]);

    delay(1000);
    WiFi.disconnect();
    delay(1000);
    WiFi.mode(WIFI_AP_STA);                  
    WiFi.softAP(ID, apPassword);             //Start HOTspot removing password will disable security
    WiFi.begin(ssid.c_str(),password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println("wifi connected");

    Serial.println(WiFi.localIP());
   
    server.on("/", handleRoot);               //Which routine to handle at root location
    server.on("/data", handleData);          //Which routine to handle at root location
    server.begin();                           //Start server

}

String field( String inStr, int n, char delim ) {
    String result = "";
    int begin = 0;
    int end = inStr.length();
    int i = 0;
    int j = 0;
    while( (j = inStr.indexOf(delim,j+1)) > 0 ){
        if( i < n ) {
            begin = j+1;
        }
        if( i == n ){
            end = j;
            break;
        }
        i++;
    }
    return inStr.substring(begin, end);
}

int writeStringToEEPROM(int addrOffset, const String &strToWrite){
    byte len = strToWrite.length();
    EEPROM.write(addrOffset, len);
    for (int i = 0; i < len; i++) {
        EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
    }
    return addrOffset + 1 + len;
}

int readStringFromEEPROM(int addrOffset, String *strToRead){
    int newStrLen = EEPROM.read(addrOffset);
    char data[newStrLen + 1];
    for (int i = 0; i < newStrLen; i++)  {
        data[i] = EEPROM.read(addrOffset + 1 + i);
    }
    data[newStrLen] = '\0';
    *strToRead = String(data);
    return addrOffset + 1 + newStrLen;
}

void setup(void){
    pixels.begin(); 
    pixels.clear(); 
    for(int i=0; i<NUMPIXELS; i++) { 
        pixels.setPixelColor(i, pixels.Color(rgbRed, rgbGreen, rgbBlue));
    }
    pixels.show(); 
    bmp280.begin(0x76);                                 // Default initialisation, place the BMP280 into SLEEP_MODE 
    bmp280.setTimeStandby(TIME_STANDBY_2000MS);     // Set the standby time to 2 seconds
    bmp280.startNormalConversion();                 // Start BMP280 continuous conversion in NORMAL_MODE  


    EEPROM.begin(512);
    Serial.begin(115200);

    int value = EEPROM.read( 0 );
    if( value != 4 ) {
      EEPROM.write(0,4);
      writeStringToEEPROM(1,settings);
      EEPROM.commit();
    }
    readStringFromEEPROM(1,&settings);


    apikey = field(settings,0,'\t');
    device = field(settings,1,'\t');
    ssid = field(settings,2,'\t');
    password = field(settings,3,'\t');
    repeat = atoi(field(settings,4,'\t').c_str());
    host = field(settings,5,'\t');
    base = field(settings,6,'\t');

  if (IMU.begin() < 0) {
    while(1) {}
  }


    connectWiFi();
}


void makePost( const int port, const char* link, const char* body ){
    Serial.print("Post reading -- ");
    uint32_t startT = millis();
    BearSSL::WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(15000);
    int retries = 60;

    while(!client.connect(host.c_str(), port) && (retries-- > 0)) {
        delay(100);
        retries--;
    }

   if(!client.connected()) {
        client.stop();
        return;
    }
//    Serial.println(body);
    
    client.print(String("POST ") + "/" + base + "/" +link + " HTTP/1.1\r\n" +
                "apikey: " + apikey + "\r\n"+
                "Host: " + host + "\r\n" +
                "Content-Type: application/json\r\n"+
                "Content-Length: "+ strlen(body) +"\r\n"+
                "\r\n"+
                body+
                "Connection: close\r\n"+
                "\r\n");  

  uint32_t to = millis() + 5000;
  if (client.connected()) {
    do {
      char tmp[32];
      memset(tmp, 0, 32);
      int rlen = client.read((uint8_t*)tmp, sizeof(tmp) - 1);
      yield();
      if (rlen < 0) {
        break;
      }
      // Only print out first line up to \r, then abort connection
      char *nl = strchr(tmp, '\r');
      if (nl) {
        *nl = 0;
        Serial.print(tmp);
        break;
      }
      Serial.print(tmp);
    } while (millis() < to);
    Serial.print(" -- ");
    Serial.println( millis()-startT );
  }

                                  
    client.stop(); 
    return; 
}

char sensorbuff[1024];
char tmpbuff[256];

float temperature, pressure, altitude;            // Create the temperature, pressure and altitude variables

void loop(void){
    server.handleClient();         
    if( millis()-tick > (repeat*1000) || tick > millis() ){
        tick = millis(); 
        if (WiFi.status() == WL_CONNECTED) {
            bmp280.getMeasurements(temperature, pressure, altitude);      
            IMU.readSensor();

              sprintf( sensorbuff, "{\"device\":\"%s\",\"values\": [ ",
                       device.c_str() );

              sprintf( tmpbuff, "{\"parameter\":\"acc-x\",\"unit\":\"m/s2\",\"value\":%f},",
                       IMU.getAccelX_mss());
              strcat (sensorbuff,tmpbuff);
              sprintf( tmpbuff, "{\"parameter\":\"acc-y\",\"unit\":\"m/s2\",\"value\":%f},",
                       IMU.getAccelY_mss());
              strcat (sensorbuff,tmpbuff);
              sprintf( tmpbuff, "{\"parameter\":\"acc-z\",\"unit\":\"m/s2\",\"value\":%f},",
                       IMU.getAccelZ_mss());
              strcat (sensorbuff,tmpbuff);
                       
              sprintf( tmpbuff, "{\"parameter\":\"gyr-x\",\"unit\":\"rad\",\"value\":%f},",
                       IMU.getGyroX_rads());
              strcat (sensorbuff,tmpbuff);
              sprintf( tmpbuff, "{\"parameter\":\"gyr-y\",\"unit\":\"rad\",\"value\":%f},",
                       IMU.getGyroY_rads());
              strcat (sensorbuff,tmpbuff);
              sprintf( tmpbuff, "{\"parameter\":\"gyr-z\",\"unit\":\"rad\",\"value\":%f},",
                       IMU.getGyroZ_rads());
              strcat (sensorbuff,tmpbuff);
                       
              sprintf( tmpbuff, "{\"parameter\":\"mag-x\",\"unit\":\"uT\",\"value\":%f},",
                       IMU.getMagX_uT());
              strcat (sensorbuff,tmpbuff);
              sprintf( tmpbuff, "{\"parameter\":\"mag-y\",\"unit\":\"uT\",\"value\":%f},",
                       IMU.getMagY_uT());
              strcat (sensorbuff,tmpbuff);
              sprintf( tmpbuff, "{\"parameter\":\"mag-z\",\"unit\":\"uT\",\"value\":%f},",
                       IMU.getMagZ_uT());
              strcat (sensorbuff,tmpbuff);
              

              strcat (sensorbuff,tmpbuff);
              sprintf( tmpbuff, "{\"parameter\":\"temperature\",\"unit\":\"C\",\"value\":%f},",
                       temperature);
              strcat (sensorbuff,tmpbuff);
              sprintf( tmpbuff, "{\"parameter\":\"pressure\",\"unit\":\"hPa\",\"value\":%f}",
                       pressure);
              strcat (sensorbuff,tmpbuff);
              
       
              
              strcat( sensorbuff, "] } " );

            makePost( 443 , "readings", sensorbuff);            
        }else{
            connectWiFi();
        }
    }
    server.handleClient();          
}
