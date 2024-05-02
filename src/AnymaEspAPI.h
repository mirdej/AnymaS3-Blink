#ifndef __ANYMA_ESP_API_INCLUDED__
#define __ANYMA_ESP_API_INCLUDED__

#include "WiFi.h"
#include "AnymaEspSettings.h"
#include "ArduinoJson.h"
#include <aWOT.h>
#include "MimeTypes.h"

extern Application app;




void cors(Request &req, Response &res)
{
  res.set("Access-Control-Allow-Origin", "*");
  res.set("Access-Control-Allow-Methods", "GET, POST, PUT, HEAD, OPTIONS");
  res.set("Access-Control-Allow-Headers", "Content-Type");
  log_v("_____________________\nPath %s, Method %d", req.path(), req.method());
  // res.status(204);
  if (req.method() == 7)
    res.sendStatus(204);
}





void setup_api()
{
  /*   app.get("/api/blink_color", &read_blink_color);
    app.post("/api/blink_color/:color", &update_blink_color); */
  // app.put("/api/blink_color/:color", &update_blink_color);
app.use(&cors);



  app.get("/api/fetchall", [](Request &req, Response &res)
          {
              JsonDocument doc;
              char data[2048];
              doc["blink_interval"] = settings.blink_interval;
              doc["blink_color"] = settings.blink_color;
              size_t len = serializeJson(doc, data);
              log_v("Data size: %d", len);
              res.status(200);
              res.set("Content-Type", "application/json");
              res.print(data); });

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
}

#endif