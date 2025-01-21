/* DATA COLLECTION SYSTEM
  Autor : TAMEKENG LANE ROMUALD
  Email: laneromuald@gmail.com
*/

/*Including External Libraries (BE)*/

//DS18B20 Sensor Management Library
#include <OneWire.h>
#include <DallasTemperature.h>

//AM2302 sensor management library
#include "DHT.h"

//RTC sensor management library
#include "RTClib.h"

//SD card management library
#include "SD.h"

// WiFi management library with ESP32
#include <WiFi.h>
#include "ThingSpeak.h"

/*Definition of communication pins*/
//Pin for DS18B20
#define ONE_WIRE_BUS 15

//Pin for HW-390
#define HSpin1 A0
#define HSpin2 A3
#define HSpin3 A6

//pin and selection for AM2302
#define DHTpin 4
#define DHTTYPE DHT22

/*Macro Definition for Date/Time*/
#define countof(a) (sizeof(a) / sizeof(a[0]))

/*Configuration of Function Instances*/
// For DS18B20 Sensor Management
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18B20(&oneWire);

// Definition of Addresses to manage each DS18B20 sensor
DeviceAddress ds18B20_1 = {0x28, 0xC5, 0x51, 0xFF, 0x62, 0x20, 0x1, 0xAC };
DeviceAddress ds18B20_2 = {0x28, 0x17, 0x56, 0x57, 0x4, 0xE1, 0x3C, 0xA5 };
DeviceAddress ds18B20_3 = {0x28, 0x24, 0xBC, 0x57, 0x4, 0xE1, 0x3C, 0x76 };

// For AM2302 Sensor Management
DHT dht(DHTpin, DHTTYPE);

// For RTC Module Management
RTC_DS3231 rtc;
DateTime now;

// For SD Card Module Management
File myFile;
const int CS = 5;
String datas;

// Deep Sleep Parameters
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for microseconds to seconds */
#define TIME_TO_SLEEP  3600         /* ESP32 sleep time (in seconds), equivalent to 60 minutes */
RTC_DATA_ATTR int bootCount = 0;   // Data saved in RTC memory

// ThingSpeak Parameters
const char* ssid1 = "XXXX";       // your network SSID (name)
const char* password1 = "XXXX";   // your network password
const char* ssid2 = "XXXX";       // your network SSID (name)
const char* password2 = "XXXX";   // your network password
WiFiClient  client;
unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "XXXX";

// Functions and Procedures

/*
  Method to print the reason why the ESP32 woke up from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("[Réveil causé par un signal externe utilisant RTC_IO]"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("[Réveil provoqué par un signal externe utilisant RTC_CNTL]"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("[Réveil provoqué par la minuterie]"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("[Réveil provoqué par le pavé tactile]"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("[Réveil causé par le programme ULP]"); break;
    default : Serial.printf("[Le réveil n'est pas dû à un sommeil profond : %d]\n", wakeup_reason); break;
  }
}

// Procedure for writing to the SD card
void WriteFile(const char * path, String message) {

  myFile = SD.open(path,  FILE_APPEND);
  if (myFile) {
    Serial.printf("Enregistrement dans %s ", path);
    myFile.println(message);
    myFile.close(); // close the file:
    Serial.println("Enregistrement éffectuer.");
  }
  
  else {
    Serial.println("Erreur d'ouverture de fichier ");
    Serial.println(path);
  }
}

// Initialization Function
void setup() {

  Serial.begin(9600);

 // Configuration of Input/Output Pins
  pinMode(HSpin1, INPUT);
  pinMode(HSpin2, INPUT);
  pinMode(HSpin3, INPUT);
  pinMode(25,OUTPUT);
  digitalWrite(25,HIGH);

  // Initialization for DS18B20 Sensor
 
  ds18B20.begin();

  // Initialization for AM2302 Sensor
  dht.begin();

 // Initialization for SD Card Module
  Serial.println("[Initializing SD card]...");
  if (!SD.begin(CS)) {
    Serial.println("[Initialization failed!]");
  }
  Serial.println("[Initialization successful].");

  // Increment the boot number and print it at every restart
  ++bootCount;
  Serial.println("[Boot number: " + String(bootCount) + " ]");

  // Reason for ESP32 wake-up
  print_wakeup_reason();

  /*
    Configure the wake-up source, Wake-up by Timer.
    We configure our ESP32 to wake up every 5 minutes.
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("[ESP32 sleeps every " + String(TIME_TO_SLEEP) + " seconds]");

  // Once Awake, we can perform our processing tasks
  Serial.println("===========================================================");

  // WiFi Initialization
  WiFi.mode(WIFI_STA);

  // ThingSpeak Initialization
  ThingSpeak.begin(client);

  // Connection or reconnection to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    for (int i = 1; i <= 5; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Connection attempt]");
        WiFi.begin(ssid1, password1);
        delay(2000);
      } else {
        i = 16;
        Serial.println("\n[Connected to WiFi]:");
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n[WiFi unavailable.]");
    }
  }
  Serial.println("-----------------------------------------");
  delay(2000);

  Serial.println("[Data collection and storage]");
 
   /*Recovering values*/

  /*Site 1 MAIZE Variety 1: CMS 8704 */
  Serial.println("[Soil data Site 1 (Maize CMS 8704)]:");
  int x1 = analogRead(HSpin1);
  Serial.print("- Soil moisture:");
  Serial.print(x1);
  Serial.println("%");
  ds18B20.requestTemperatures(); // Send the command to get temperatures
  Serial.print("- Ground temperature:");
  Serial.print(ds18B20.getTempC(ds18B20_1));
  Serial.println("°C");
  Serial.println("-----------------------------------------");

  /*Site 2 CORN Variety 2: CHH 101*/
  Serial.println("[Soil Data Site 2 (Corn CHH 101)]:");
  int x2 = analogRead(HSpin2);
  Serial.print("- Soil Moisture: ");
  Serial.print(x2);
  Serial.println("%");
  Serial.print("- Soil Temperature: ");
  Serial.print(ds18B20.getTempC(ds18B20_3));
  Serial.println("°C");
  Serial.println("-----------------------------------------");


    /*Site 3 WATERMELON*/
  Serial.println("[Soil Data Site 3 (Watermelon Crympson)]:");
  int x3 = analogRead(HSpin3);
  Serial.print("- Soil Moisture: ");
  Serial.print(x3);
  Serial.println("%");
  Serial.print("- Soil Temperature: ");
  Serial.print(ds18B20.getTempC(ds18B20_2));
  Serial.println("°C");
  Serial.println("-----------------------------------------");
  delay(2000);

  Serial.println("[Meteorological Data on Site]:");

  float HR = dht.readHumidity();
  float TA = dht.readTemperature();
  // Check if a reading failed and display a message
  if (isnan(HR) || isnan(TA)) {
    Serial.println(F("[Failed to read from AM2302 sensor!]"));
  }
  Serial.print(F("- Relative Humidity: "));
  Serial.print(HR);
  Serial.println("%");
  Serial.print(F("- Air Temperature: "));
  Serial.print(TA);
  Serial.println(F("°C"));
  Serial.println("-----------------------------------------");
  delay(4000);

  // Retrieve the date in a unique format
  char datestring[20];
  now = rtc.now();
  snprintf_P(datestring,
            countof(datestring),
            PSTR("%02u/%02u/%04u,%02u:%02u:%02u"),
            now.day(),
            now.month(),
            now.year(),
            now.hour(),
            now.minute(),
            now.second());

  // Local Data Storage
  datas = String(datestring) + "," + String(x1) + "," + String(ds18B20.getTempC(ds18B20_1)) + "," + String(HR) + "," + String(TA);
  WriteFile("/Site_1.txt", datas);
  datas = String(datestring) + "," + String(x2) + "," + String(ds18B20.getTempC(ds18B20_3)) + "," + String(HR) + "," + String(TA);
  WriteFile("/Site_2.txt", datas);
  datas = String(datestring) + "," + String(x3) + "," + String(ds18B20.getTempC(ds18B20_2)) + "," + String(HR) + "," + String(TA);
  WriteFile("/Site_3.txt", datas);

  // Attempt to connect to another WiFi Access Point if the current one is unavailable
  if (WiFi.status() != WL_CONNECTED) {
    for (int i = 1; i <= 5; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Connection Attempt]");
        WiFi.begin(ssid2, password2);
        delay(2000);
      } else {
        i = 16;
        Serial.println("\n[Connected to WiFi]:");
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n[WiFi Unavailable.]");
      WriteFile("/Log.txt", String(datestring) + ": [WiFi Unavailable..]");
      Serial.println("----------------------------------------------------------");
    }
  }

 // Sending the value of each sensor with ThingSpeak.setField
ThingSpeak.setField(1, float(x1));
ThingSpeak.setField(2, ds18B20.getTempC(ds18B20_1));
ThingSpeak.setField(3, float(x2));
ThingSpeak.setField(4, ds18B20.getTempC(ds18B20_3));
ThingSpeak.setField(5, float(x3));
ThingSpeak.setField(6, ds18B20.getTempC(ds18B20_2));
ThingSpeak.setField(7, HR);
ThingSpeak.setField(8, TA);

// Sending the entire packet to ThingSpeak. There are up to 8 fields in a channel,
// which allows storing up to 8 different pieces of information in a channel.
int code = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
if (code == 200) {
    Serial.println("[Channel update successful.]");
} else {
    Serial.println("Channel update issue. HTTP error code " + String(code));
    WriteFile("/Log.txt", String(datestring) + " : [Channel update issue. HTTP error code " + String(code) + "]");
}
Serial.println("=========================================");

/*
    Once the processing is completed, put the ESP32 in Deep Sleep Mode
*/
digitalWrite(25, LOW);
Serial.println("[Entering Deep Sleep Mode]");
Serial.flush();
esp_deep_sleep_start();
// Everything below this point will not execute or display

}

// Loop Function
void loop() {
  // The Loop function is not used in Deep Sleep Mode
}
