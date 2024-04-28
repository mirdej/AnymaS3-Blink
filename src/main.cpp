/*========================================================================================

[ a n y m a ]
S3 BLINK

or: worlds most complicated Arduino Blink Sketch


© 2024 Michael Egger AT anyma.ch

==========================================================================================*/
//                                                                                      LIB
#include <Arduino.h>
#include <Preferences.h>

#include "FS.h"
#include <LittleFS.h>

#include <ArduinoJson.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#include "ESP-FTP-Server-Lib.h"
#include "FTPFilesystem.h"

#include "web_api.h"
//----------------------------------------------------------------------------------------
//                                                                                  CONFIG
#define FTP_USER "me"
#define FTP_PASSWORD "me"

#define MAIN_FILE_SYSTEM LittleFS

#define PIN_PIX 17
#define NUM_PIXELS 1

const char *NTP_SERVER = "pool.ntp.org";
const char *TZ_STRING = "CET-1CEST,M3.5.0/2,M10.5.0/3";

//----------------------------------------------------------------------------------------
//                                                                                 GLOBALS

WiFiMulti wifiMulti;
FTPServer ftp;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

CRGB pixel[NUM_PIXELS];

bool leds_changed;

Preferences preferences;
bool preferences_need_write;

//----------------------------------------------------------------------------------------
//                                                                  GLOBALS EXPOSED IN API

int blink_interval;
CRGB blink_color = CRGB::Green;
String hostname = "baby-s3";

//----------------------------------------------------------------------------------------
//																				                                        LED Task

void led_task(void *)
{

  FastLED.addLeds<SK6812, PIN_PIX, GRB>(pixel, NUM_PIXELS);
  FastLED.setBrightness(100);

  while (1)
  {
    if (leds_changed)
    {
      FastLED.show();
      leds_changed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(40)); // ~25 fps display
  }

  vTaskDelete(NULL); // we never get here
}

//========================================================================================
//----------------------------------------------------------------------------------------
//																				                                      NETWORKING

//----------------------------------------------------------------------------------------
//                                                                                  Server
void setup_webserver()
{
  if (!MDNS.begin(hostname.c_str()))
  {
    log_e("Error setting up MDNS responder!");
  }
  else
  {
    log_i("mDNS responder started");
    log_i("Hostname: %s", hostname);
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ftp", "tcp", 21);
  }

  // to avoid cross-site-injection errors when testing webpage locally
  // see:https://stackoverflow.com/questions/71910612/how-do-i-solve-esp32-async-webserver-cors-error
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  server.onNotFound([](AsyncWebServerRequest *request)
                    {if (request->method() == HTTP_OPTIONS) {
                          request->send(200);
                    } else {
                          request->send(404, "application/json", "{\"message\":\"Not found\"}");
  } });
  setup_api(); // see web_api.h

  server.serveStatic("/", MAIN_FILE_SYSTEM, "/").setDefaultFile("index.html");
  // server.addHandler(&ws);

  server.begin();
  log_i("Webserver started.");

  ftp.addUser(FTP_USER, FTP_PASSWORD);
  ftp.addFilesystem("LittleFS", &LittleFS);
  ftp.begin();
  log_i("FTP server started.");
}

//----------------------------------------------------------------------------------------
//																				                                       WiFi Task

void wifi_task(void *)
{
  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_STRING, 1);
  tzset();

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // better resonsiveness, more power consumption
  wifiMulti.addAP("Anymair", "Mot de passe pas complique");

  while (WiFi.status() != WL_CONNECTED)
  {
    wifiMulti.run();
    vTaskDelay(pdMS_TO_TICKS(250));
  }
  log_i("IP address: %s", WiFi.localIP().toString().c_str());

  setup_webserver();

  while (1)
  {

    if (WiFi.status() == WL_CONNECTED)
    {
      ftp.handle();
      taskYIELD();
    }
    else
    {
      Serial.println("[WIFI] Connecting");
      WiFi.reconnect();
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

//----------------------------------------------------------------------------------------
//																				                                Preferences Task

void preferences_task(void *)
{
  preferences.begin("anyma");
  blink_interval = preferences.getInt("blink_interval", 1000);
  hostname = preferences.getString("hostname", "baby-s3");
  if (preferences.getBytesLength("blink_color") == sizeof(blink_color))
  {
    preferences.getBytes("blink_color", &blink_color, sizeof(blink_color));
  }

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(30000));
    if (preferences_need_write)
    {
      preferences.putInt("blink_interval", blink_interval);
      preferences.putString("hostname", hostname);
      preferences.putBytes("blink_color", &blink_color, sizeof(blink_color));
      preferences_need_write = false;
      log_v("Preferences written");
    }
  }
}

//----------------------------------------------------------------------------------------
//																				                                      Blink Task
void blink_task(void *)
{
  bool led_on;

  while (1)
  {
    if (led_on)
    {
      pixel[0] = CRGB::Black;
      led_on = false;
    }
    else
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        pixel[0] = blink_color;
      }
      else
      {
        pixel[0] = CRGB::Red;
      }
      led_on = true;
    }
    leds_changed = true;
    vTaskDelay(pdMS_TO_TICKS(blink_interval));
  }
}

//========================================================================================
//----------------------------------------------------------------------------------------
//																				                                           Setup

void setup()
{
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1000));
  log_v("%s",PROJECT_PATH);
  log_v("Version %s",FIRMWARE_VERSION);

  log_v("________________________");
  log_v("Setup");

  // LittleFS.begin();
  // Format if there is no Filesystem, Max open files = 10 for better Webserver stability
  LittleFS.begin(true, "/littlefs", 10U);

  xTaskCreate(
      preferences_task, // Function that implements the task.
      "PREFS Task",     // Text name for the task.
      8192,             // Stack size in words, not bytes.
      NULL,             // Parameter passed into the task.
      0,                // Priority at which the task is created.
      NULL);

  xTaskCreate(
      wifi_task,    // Function that implements the task.
      "WIFI-Setup", // Text name for the task.
      8192,         // Stack size in words, not bytes.
      NULL,         // Parameter passed into the task.
      0,            // Priority at which the task is created.
      NULL);

  xTaskCreate(
      led_task,   // Function that implements the task.
      "LED Task", // Text name for the task.
      8192,       // Stack size in words, not bytes.
      NULL,       // Parameter passed into the task.
      1,          // Priority at which the task is created.
      NULL);

  xTaskCreate(
      blink_task, // Function that implements the task.
      "Blink",    // Text name for the task.
      8192,       // Stack size in words, not bytes.
      NULL,       // Parameter passed into the task.
      0,          // Priority at which the task is created.
      NULL);

  log_v("Setup Done");
  log_v("________________________");
}

//========================================================================================
//----------------------------------------------------------------------------------------
//																				Loop

void loop()
{
}
