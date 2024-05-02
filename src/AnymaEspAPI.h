#ifndef __ANYMA_ESP_API_INCLUDED__
#define __ANYMA_ESP_API_INCLUDED__

#include "WiFi.h"
#include "AnymaEspSettings.h"
#include "ArduinoJson.h"
#include <aWOT.h>
#include "MimeTypes.h"

extern Application app;

// set to 1 to be able to run a development server on a remote machine
// adds headersfor CORS
/*Cross-origin resource sharing (CORS) is a mechanism for integrating applications.
CORS defines a way for client web applications that are loaded in one domain to interact with resources in a different domain.
*/
#define __APP_USE_CORS_HEADERS 1

String sanitizeColorInput(String s)
{
  char buf[7];
  s.toLowerCase();
  s.toCharArray(buf, 6);
  for (int i = 0; i < 6; i++)
  {
    if ((buf[i] < '0' || buf[i] > '9') &&
        (buf[i] < 'a' || buf[i] > 'f'))
    {
      buf[i] = '0';
    }
  }
  buf[6] = 0;
  return String(buf);
}

void setup_api()
{
  //--------------------------------------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------------------------------- GET

  app.get("/api/settings", [](Request &req, Response &res)
          {
              res.status(200);
              res.set("Content-Type", "application/json");
              serializeJson(settings.get_json(), req); });

  app.get("/api/deviceinfo", [](Request &req, Response &res)
          {
              JsonDocument doc;
              char data[2048];
              doc["message"] = "deviceinfo";
              doc["firmware"] = PROJECT_PATH;
              doc["version"] = FIRMWARE_VERSION;

              doc["chip"] = ESP.getChipModel();
              doc["chip_rev"] = ESP.getChipRevision();
              doc["chip_cores"] = ESP.getChipCores();
              doc["chip_fcpu"] = ESP.getCpuFreqMHz();
              doc["sdk_version"] = ESP.getSdkVersion();

              doc["flash_size"] = ESP.getFlashChipSize();
              doc["flash_speed"] = ESP.getFlashChipSpeed();
              doc["flash_mode"] = ESP.getFlashChipMode();
              doc["flash_free_sketch_space"] = ESP.getFreeSketchSpace();

              doc["hostname"] = settings.hostname;
              doc["ip"] = WiFi.localIP().toString();
              doc["mac"] = WiFi.macAddress();
              doc["millis"] = millis();

              multi_heap_info_t info;
              heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

              doc["ram_free"] = info.total_free_bytes;
              doc["ram_lowest"] = info.minimum_free_bytes;
              doc["ram_largest_free_block"] = info.largest_free_block;

              doc["psram_size"] = ESP.getPsramSize();
              doc["psram_free"] = ESP.getFreePsram();
              doc["psram_max_alloc"] = ESP.getMaxAllocPsram();

              doc["fs_used"] = LittleFS.usedBytes();
              doc["fs_total"] = LittleFS.totalBytes();

              size_t len = serializeJson(doc, data);
              log_v("Data size: %d", len);
              res.status(200);
              res.print(data); });

  //--------------------------------------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------------------------------- PUT

  app.put("/api/settings", [](Request &req, Response &res)
          {
              JsonDocument doc;
              DeserializationError error = deserializeJson(doc, req);
              // Test if parsing succeeds.
              if (error)
              {
                log_e("DeserializeJson() failed: %s", error.f_str());
                res.sendStatus(500);
                return;
              }
              //serializeJson(doc, Serial);

              int i = doc["params"]["blink_interval"].as<int>();
              if (i) {
                i = constrain(i,50,60000);
                settings.blink_interval = i;
              }

              if (doc["params"]["blink_color"]) {
                String s = doc["params"]["blink_color"].as<String>();
                log_v("COLOR %s, %s",s.c_str(),sanitizeColorInput(s).c_str());
                settings.blink_color = sanitizeColorInput(s);
              }

              if (doc["params"]["hostname"]) {
                  settings.hostname = doc["params"]["hostname"].as<String>();
              }
              settings.dirty = true;
              res.sendStatus(204); });
}

#endif