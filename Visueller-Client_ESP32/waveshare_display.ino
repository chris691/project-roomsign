#include <GxEPD2_BW.h>
#include "GxEPD2_display_selection_new_style.h"
#include <Arduino.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <WiFi.h>
#include <ESP32Time.h>
#include <NTPClient.h>
#include <iostream>
#include <sstream>
#include <ctime>

#include <HTTPClient.h>
#include <ArduinoJson.h>

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
#define ENABLE_GxEPD2_GFX 0
#define MY_NTP_SERVER "at.pool.ntp.org"
#define MY_TZ "CET,M3.31.0/02,M10.27.0/03"


const char* ssid = "***";         
const char* password = "***";  
ESP32Time rtc(3600);                           
time_t now;                                    
tm tm;                                          
char oldMin;
String Date;
String roomnumber = "I012";
String raumnutzung = "Hörsaal";
JsonArray lessons;


void setup() {
  display.init();
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  display.setRotation(1);
  u8g2Fonts.begin(display);  
  delay(1000);
  uint16_t bg = GxEPD_WHITE;
  uint16_t fg = GxEPD_BLACK;
  u8g2Fonts.setForegroundColor(fg);  
  u8g2Fonts.setBackgroundColor(bg);
  connectToWifi();
  oldMin = rtc.getMinute();
  Date = rtc.getDate();
  printTime();
  printDate();
  initDisplay();
  // WiFi.getSleep();
}

// Daten vom Server beziehen und sortieren
void getDataFromBackend() {
  HTTPClient http;
  String weekday = getWeekday();
  http.begin("https://roomsign-api.schultz-tech.de/api/lessonForTheDay?roomnumber=" + roomnumber + "&weekday=" + weekday);  // API-Endpunkt
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String payload = http.getString();
    const char* responseText = payload.c_str();
    const size_t maxEntries = 10;
    const size_t bufferSize = JSON_ARRAY_SIZE(maxEntries) + maxEntries * JSON_OBJECT_SIZE(6) + 320;
    DynamicJsonDocument doc(bufferSize);
    DeserializationError error = deserializeJson(doc, responseText);
    if (error) {
      Serial.print("Fehler beim Parsen des JSON-Textes: ");
      Serial.println(error.c_str());
      return;
    }
    lessons = doc["lessons"];
    int amountOfLessons = 0;
    for (JsonObject lesson : lessons) {
      const char* roomnumber = lesson["roomnumber"];
      const char* day = lesson["day"];
      const char* starting_time = lesson["starting_time"];
      const char* ending_time = lesson["ending_time"];
      const char* lecturer = lesson["lecturer"];
      const char* module = lesson["module"];
      if (lesson["starting_time"] == "08:00:00" || lesson["starting_time"]== "08:45:00") {
        lesson["pos"] = "0";
      } else if (lesson["starting_time"] == "09:45:00") {
        lesson["pos"] = "1";
      } else if (lesson["starting_time"] == "11:30:00") {
        lesson["pos"] = "2";
      } else if (lesson["starting_time"] == "13:30:00") {
        lesson["pos"] = "3";
      } else if (lesson["starting_time"] == "15:15:00") {
        lesson["pos"] = "4";
      } else if (lesson["starting_time"] == "17:00:00") {
        lesson["pos"] = "5";
      } else if (lesson["starting_time"] == "18:45:00") {
        lesson["pos"] = "6";
      } else if (lesson["starting_time"] == "20:15:00") {
        lesson["pos"] = "7";
      }
      const char* position = lesson["pos"];
      int posAsInt = std::stoi(position);
      if (amountOfLessons < posAsInt) {
        amountOfLessons = posAsInt;
      }
    }
    printTimetable(amountOfLessons + 1);
  } else {
    Serial.print("HTTP Request failed with error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}



void connectToWifi() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }
  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  configTime(0, 0, MY_NTP_SERVER);  
  setenv("TZ", MY_TZ, 1);           
  tzset();
  // rtc.setTime(20, 59, 22, 18, 9, 2024); // Zeit zum Testen
  setTime();
}
//
void setTime() {
  time(&now);
  localtime_r(&now, &tm);
  int sec = tm.tm_sec;
  int min = tm.tm_min;
  int hour = tm.tm_hour;
  int day = tm.tm_mday;
  int mon = tm.tm_mon + 1;
  int year = tm.tm_year + 1900;
  rtc.setTime(sec, min, hour, day, mon, year);
}

void initDisplay() {
  getDataFromBackend();
  printRoomInfo();
}

void printTime() {
  oldMin = rtc.getMinute();  
  do {
    u8g2Fonts.setFont(u8g2_font_inr33_mr);  
    u8g2Fonts.setCursor(320, 50);           
    u8g2Fonts.print(rtc.getTime("%H:%M")); 
  } while (display.nextPage());             
}

void printRoomInfo() {
  u8g2Fonts.setFont(u8g2_font_logisoso58_tr);  
  do {
    u8g2Fonts.setCursor(10, 100);  
    u8g2Fonts.print(roomnumber);   
  } while (display.nextPage());   
  u8g2Fonts.setFont(u8g2_font_helvB24_te);
  do {
    u8g2Fonts.setCursor(180, 100);  
    u8g2Fonts.print(raumnutzung); 
  }while (display.nextPage());
}

void printDate() {
  Date = rtc.getDate();                   
  u8g2Fonts.setFont(u8g2_font_fub25_tr); 
  do {
    u8g2Fonts.setCursor(320, 100);          
    u8g2Fonts.print(getWeekday());         
    u8g2Fonts.setCursor(380, 100);         
    u8g2Fonts.print(rtc.getTime("%d.%m"));  
  } while (display.nextPage());             
}

// Wochentag bestimmen
String getWeekday() {
  int dayofTheWeek = 0;               
  dayofTheWeek = rtc.getDayofWeek();  
  String wdn;                         
  switch (dayofTheWeek) {             
    case 0:
      wdn = "So";  // Sonntag
      break;
    case 1:
      wdn = "Mo";  // Montag
      break;
    case 2:
      wdn = "Di";  // Dienstag
      break;
    case 3:
      wdn = "Mi";  // Mittwoch
      break;
    case 4:
      wdn = "Do";  // Donnerstag
      break;
    case 5:
      wdn = "Fr";  // Freitag
      break;
    case 6:
      wdn = "Sa";  // Samstag
      break;
  }
  Serial.print("wdn: ");
  Serial.println(wdn);
  return wdn;  
}
// Zeiten auf der linken Seite Zeichnen
void printTimetable(int stunden) {
  Serial.print("bin im timetable!");
  u8g2Fonts.setFont(u8g2_font_helvB14_te);  
  int fontHeight = u8g2Fonts.getFontAscent() + u8g2Fonts.getFontDescent();
  int fontspace = 100 / 70;
  int totalFontHeight = fontHeight * 1.6;
  do {
    if (stunden == 0) {
      centerText("Für diesen Raum sind heute keine Raumbelegung vorhanden.", 150, 300, 10, totalFontHeight, 300, totalFontHeight * 4);
      return;
    }

    int y = 1;
    String time = "08:00 - 09:30";
    Serial.println((totalFontHeight * 4 * (8.0 / stunden)));
    int lessonCounter = 0;
    for (int i = 125; i <= 582; i += (totalFontHeight * 4 * (8.0 / stunden))) {
      switch (y) {  
        case 1:
          time = "08:00 - 09:30";
          if (rtc.getTime("%H:%M") >= "08:00") {
          }
          break;
        case 2:
          time = "09:45 - 11:15";
          break;
        case 3:
          time = "11:30 - 13:00";
          break;
        case 4:
          time = "13:30 - 15:00";
          break;
        case 5:
          time = "15:15 - 16:45";
          break;
        case 6:
          time = "17:00 - 18:30";
          break;
        case 7:
          time = "18:45 - 20:15";
          break;
        case 8:
          time = "20:15 - 21:45";
          break;
      }
      centerText(String(y) + ".", 50, i, 10, totalFontHeight, 50, totalFontHeight * 4);
      centerText(time, 10, i + totalFontHeight, 10, totalFontHeight, 100, totalFontHeight * 4);
      display.fillRect(0, i - 5, 600, 5, GxEPD_BLACK);
      printLesson(i, totalFontHeight, lessonCounter);
      display.fillRect(0, 120, 600, 10, GxEPD_BLACK);
      display.fillRect(110, 130, 5, 600, GxEPD_BLACK);
      y++;
      lessonCounter = lessonCounter + 1;
    }
  } while (display.nextPage());
}

// Hilfs-Funktion zum Zeichnen auf dem E-Paper-Display
void centerText(String text, int xStart, int yStart, int textHeight, int lineHeight, int maxLineWidth, int fildHeight) {
  int x = xStart;
  int y = yStart + ((fildHeight - textHeight) / 2);
  int currentLineWidth = 0;
  for (int i = 0; i < text.length(); i++) {
    char currentChar = text.charAt(i);  
    char charString[2];                 
    charString[0] = currentChar;        
    charString[1] = '\0';              
    int charWidth = u8g2Fonts.getUTF8Width(charString);  
    if (currentLineWidth + charWidth > maxLineWidth) {
      x = xStart;
      y += lineHeight;
      currentLineWidth = 0;  
    }
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(text.charAt(i));
    currentLineWidth += charWidth;
    x += charWidth;
  }
}
// Vorlesungen vorbereiten für das Zeichnen
void printLesson(int y, int totalFontHeight, int lessonCounter) {
  int counter = 0;
  for (JsonObject lesson : lessons) {
    Serial.print("das ist in der PrintLesson-Funktion: ");
    const char* module = lesson["module"];
    const char* lecturer = lesson["lecturer"];
    const char* course_of_study = lesson["course_of_study"];
    const char* posStr = lesson["pos"];
    Serial.print(module);
    Serial.print(posStr);
    Serial.println(lessonCounter);
    int pos = atoi(posStr);
    if (lessonCounter == pos && counter == 0) {
      centerText(module, 150, y, 10, totalFontHeight, 250, totalFontHeight * 4);
      centerText(lecturer, 150, y + totalFontHeight + 5, 10, totalFontHeight, 250, totalFontHeight * 4);
      centerText(course_of_study, 150, y + (2 * totalFontHeight) + 5, 10, totalFontHeight, 250, totalFontHeight * 4);
      counter++;
    } else if (lessonCounter == pos && counter == 1) {
      centerText(module, 300, y, 10, totalFontHeight, 400, totalFontHeight * 4);
      centerText(lecturer, 300, y + totalFontHeight + 5, 10, totalFontHeight, 400, totalFontHeight * 4);
      centerText(course_of_study, 300, y + (2 * totalFontHeight) + 5, 10, totalFontHeight, 400, totalFontHeight * 4);
      counter++;
    } else if (lessonCounter == pos && counter == 2) {
    }
  }
}

void loop() {
  // True sobald die Zeit eine Min weiter geht
  if (oldMin != rtc.getMinute()) {
    display.setPartialWindow(320, 0, 300, 50);
    display.firstPage();
    printTime();
  }
  // True sobald ein neuer Tag beginnt
  if (Date != rtc.getDate() && rtc.getSecond() > 5) {
    display.setPartialWindow(320, 60, 300, 50);
    display.firstPage();
    printDate();
    display.setPartialWindow(0, 125, 480, 523);
    getDataFromBackend();
  }
}
