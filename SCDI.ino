/* DISPOSITIF DE COLLECTE DE DONNEE
  Auteur : TAMEKENG LANE ROMUALD
  Email: laneromuald@gmail.com
*/

/*Inclision des Bibliothèques Externes (BE)*/

//Bibliothèque pour gestion des capteurs DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

//Bibliothèque pour gestion du capteur AM2302
#include "DHT.h"

//Bibliothèque pour gestion du module RTC
#include "RTClib.h"

//Bibliothèque pour gestion de la Carte SD
#include "SD.h"

// Bibliothèque de gestion du WiFi avec ESP32
#include <WiFi.h>
#include "ThingSpeak.h"

/*Définition des Broches de communication*/
//Broche pour DS18B20
#define ONE_WIRE_BUS 15

//Adresse pour chaque capteur DS18B20
//Capteur 1: 28 C5 51 FF 62 20 1 AC
//Capteur 2:28 17 56 57 4 E1 3C A5
//Capteur 3:28 24 BC 57 4 E1 3C 76

//Broche pour HW-390
#define HSpin1 A0
#define HSpin2 A3
#define HSpin3 A6

//Broche et choix pour AM2302
#define DHTpin 4
#define DHTTYPE DHT22

/*Définition de la macro Date/Heure*/
#define countof(a) (sizeof(a) / sizeof(a[0]))

/*Configuration des instances de fonctions*/
//Pour Gestion des Capteurs DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18B20(&oneWire);

//Définition des Adresses pour gérer chaque capteur DS18B20
DeviceAddress ds18B20_1 = {0x28, 0xC5, 0x51, 0xFF, 0x62, 0x20, 0x1, 0xAC };
DeviceAddress ds18B20_2 = { 0x28, 0x17, 0x56, 0x57, 0x4, 0xE1, 0x3C, 0xA5 };
DeviceAddress ds18B20_3 = { 0x28, 0x24, 0xBC, 0x57, 0x4, 0xE1, 0x3C, 0x76 };

//Pour gestion du capteur AM2302
DHT dht(DHTpin, DHTTYPE);

//Pour gestion du module RTC
RTC_DS3231 rtc;
DateTime now;

//Pour Gestion du module Carte SD
File myFile;
const int CS = 5;
String datas;

//Parmètres du Deep Sleep
#define uS_TO_S_FACTOR 1000000ULL  /* Facteur de conversion des microsecondes en secondes */
#define TIME_TO_SLEEP  300     /* Temps de mise en veille de l'ESP32 (en secondes), Soit 5 min */
RTC_DATA_ATTR int bootCount = 0; // Enregistrement des données sur le mémoire RTC

// Paramètres ThingSpeak
const char* ssid1 = "XXXX";   // your network SSID (name)
const char* password1 = "XXXX";   // your network password
const char* ssid2 = "XXXX";   // your network SSID (name)
const char* password2 = "XXXX";   // your network password
WiFiClient  client;
unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "XXXX";


// Fonctions et Procédure utiles
/*
  Méthode pour imprimer la raison pour laquelle l'ESP32 a été réveillé de son sommeil
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

// Procédure d'écriture dans la Carte SD
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

//Fonction d'initialisation
void setup() {

  Serial.begin(9600);
  //Configuration des Broches d'Entrée/Sortie
  pinMode(HSpin1, INPUT);
  pinMode(HSpin2, INPUT);
  pinMode(HSpin3, INPUT);
  pinMode(25,OUTPUT);
  digitalWrite(25,HIGH);
  // Initialisation pour Capteur DS18B20
  ds18B20.begin();

  // Initialisation pour Capteur AM2302
  dht.begin();

  //Initialisation pour Module RTC
  if (! rtc.begin()) {
    Serial.println("[Impossible de trouver le RTC]");
    // while (1);
    delay(5000);
    rtc.begin();
  }
  rtc.adjust(DateTime(__DATE__, __TIME__));

  //Initialisation pour Module Carte SD
  Serial.println("[Initialisation de la carte SD]...");
  if (!SD.begin(CS)) {
    Serial.println("[L'initialisation a échoué !]");
  }
  Serial.println("[Initialisation effectuée].");
  //Incrémenter le numéro de démarrage et l'imprimer à chaque redémarrage
  ++bootCount;
  Serial.println("[Numéro de démarrage : " + String(bootCount) + " ]");
  //La raison du réveil de l'ESP32
  print_wakeup_reason();
  /*
    Configuration la source de réveil, Réveil par Minuterie.
    Nous configurons notre ESP32 pour qu'il se réveille toutes les 30 Min.
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("[ESP32 en veille à chaque " + String(TIME_TO_SLEEP) + " Secondes]");

  // Une fois Réveillé nous Pouvons effectuer nos Traitements
  Serial.println("===========================================================");
  //Initialisation WiFi
  WiFi.mode(WIFI_STA);

  // Initialisation de ThingSpeak
  ThingSpeak.begin(client);

  // Connection ou reconnection au WiFi
  if (WiFi.status() != WL_CONNECTED) {
    for (int i = 1; i <= 5; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Tentative de connexion]");
        WiFi.begin(ssid1, password1);
        delay(2000);
      }
      else {
        i = 16;
        Serial.println("\n[Connecté Au WiFi] :");

      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n[PA WiFi Indisponible.]");
    }
  }
  Serial.println("-----------------------------------------");
  delay(2000);
  Serial.println("[Collecte et Stockages des Données]");
  /*Récupération des valeurs*/

  /*Site 1 MAIS Varièté 1 : CMS 8704 */
  Serial.println("[Données Pédologiques Site 1 (Maïs CMS 8704)] :");
  int x1 = analogRead(HSpin1);
  Serial.print("- Humidité du sol:");
  Serial.print(x1);
  Serial.println("%");
  ds18B20.requestTemperatures(); // Send the command to get temperatures
  Serial.print("- Température du sol:");
  Serial.print(ds18B20.getTempC(ds18B20_1));
  Serial.println("°C");
  Serial.println("-----------------------------------------");

  /*Site 2 MAIS Varièté 2 : CHH 101*/
  Serial.println("[Données Pédologiques Site 2 (Maïs CHH 101)] :");
  int x2 = analogRead(HSpin2);
  Serial.print("- Humidité du sol:");
  Serial.print(x2);
  Serial.println("%");
  Serial.print("- Température du sol:");
  Serial.print(ds18B20.getTempC(ds18B20_3));
  Serial.println("°C");
  Serial.println("-----------------------------------------");


  /*Site 3 PASTEQUE*/
  Serial.println("[Données Pédologiques Site 3 (Pastèque Crympson)] :");
  int x3 = analogRead(HSpin3);
  Serial.print("- Humidité du sol:");
  Serial.print(x3);
  Serial.println("%");
  Serial.print("- Température du sol:");
  Serial.print(ds18B20.getTempC(ds18B20_2));
  Serial.println("°C");
  Serial.println("-----------------------------------------");
  delay(2000);

  Serial.println("[Données Météorologique sur Site] :");
  // La lecture de la température ou de l'humidité prend environ 250 millisecondes !
  // Les relevés du capteur peuvent également être "vieux" de 2 secondes (c'est un capteur très lent).
  float HR = dht.readHumidity();
  float TA = dht.readTemperature();
  // On Vérifie si une lecture a échoué et on affiche un message.
  if (isnan(HR) || isnan(TA)) {
    Serial.println(F("[Échec de la lecture du capteur AM2302 !]"));
  }
  Serial.print(F("- Humidité Relative:"));
  Serial.print(HR);
  Serial.println("%");
  Serial.print(F("- Température de l'Air:"));
  Serial.print(TA);
  Serial.println(F("°C "));
  Serial.println("-----------------------------------------");
  delay(4000);

  //Récupération de la date dans un format unique
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
             now.second() );
  //Stockage Local des données
  datas = String(datestring) + "," + String(x1) + "," + String(ds18B20.getTempC(ds18B20_1)) + "," + String(HR) + "," + String(TA);
  WriteFile("/Site_1.txt", datas);
  datas = String(datestring) + "," + String(x2) + "," + String(ds18B20.getTempC(ds18B20_3)) + "," + String(HR) + "," + String(TA);
  WriteFile("/Site_2.txt", datas);
  datas = String(datestring) + "," + String(x3) + "," + String(ds18B20.getTempC(ds18B20_2)) + "," + String(HR) + "," + String(TA);
  WriteFile("/Site_3.txt", datas);

  //Essaie d'accès à un autre Point d'Accès WiFi en cas d'Indisponibilité de l'autre
  if (WiFi.status() != WL_CONNECTED) {
    for (int i = 1; i <= 5; i++) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Tentative de connexion]");
        WiFi.begin(ssid2, password2);
        delay(2000);
      }
      else {
        i = 16;
        Serial.println("\n[Connecté Au WiFi]:");
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n[WiFi Indisponible.]");
      WriteFile("/Log.txt", String(datestring) + ": [WiFi Indisponible..] " );
      Serial.println("----------------------------------------------------------");
    }
  }
  //Envoie de la valeur de chaque capteur avec ThingSpeak.setField
  ThingSpeak.setField(1, float(x1));
  ThingSpeak.setField(2, ds18B20.getTempC(ds18B20_1));
  ThingSpeak.setField(3, float(x2));
  ThingSpeak.setField(4, ds18B20.getTempC(ds18B20_3));
  ThingSpeak.setField(5, float(x3));
  ThingSpeak.setField(6, ds18B20.getTempC(ds18B20_2));
  ThingSpeak.setField(7, HR );
  ThingSpeak.setField(8, TA);

  //Envoie de tous le pacquet sur ThingSpeak. Il y a jusqu'à 8 champs dans un canal,
  //ce qui permet de stocker jusqu'à 8 informations différentes dans un canal.
  int code = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (code == 200) {
    Serial.println("[Mise à jour du canal réussie.]");
  }
  else {
    Serial.println("Problème de mise à jour du canal. Code d'erreur HTTP " + String(code));
    WriteFile("/Log.txt", String(datestring) + " : [Problème de mise à jour du canal. Code d'erreur HTTP " + String(code) + "]");
  }
  Serial.println("=========================================");

  /*
      Une Fois les traitement terminés on Fait Passer L'ESP32 en Mode Deep Mode
  */
  digitalWrite(25,LOW);
  Serial.println("[Départ pour Mode Deep Sleep]");
  Serial.flush();
  esp_deep_sleep_start();
  //Tous ce qui sera ici ne pourra pas s'afficher, ne pourra pas être éxécuté
}

//Fonction de Boucle
void loop() {
  //La fonction Loop N'est Utile en Mode Deep Sleep
}
