//----------------------------------------------------------------------------------------
//                                                                       EXPOSED VARIABLES

extern int blink_interval;
extern CRGB blink_color;
extern String hostname;

//----------------------------------------------------------------------------------------

String colorToHex(CRGB col);
CRGB hexToColor(String s);
extern AsyncWebServer server;
extern bool preferences_need_write;

//----------------------------------------------------------------------------------------
//                                                                                  WEBAPI

void setup_api()
{
    server.on("/api/fetchall", HTTP_GET, [](AsyncWebServerRequest *request)
              {           
                JsonDocument doc;
                char data[2048];
                doc["blink_interval"] = blink_interval;
                doc["blink_color"] = colorToHex(blink_color);
                size_t len = serializeJson(doc, data);
                log_v("Data size: %d", len);
                request->send(200, "text/text", data); });

    server.on("/api/blink_color", HTTP_GET, [](AsyncWebServerRequest *request)
              {
            log_v("api_blink_color");
              if (request->hasParam("set"))
              {
                blink_color = hexToColor(request->getParam("set")->value());
                request->send(200, "text/text", "ok");
                preferences_need_write = true;
              }else {
              request->send(200, "text/text", colorToHex(blink_color));} });

    server.on("/api/blink_interval", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              if (request->hasParam("set"))
              {
                blink_interval = request->getParam("set")->value().toInt();
                blink_interval = constrain(blink_interval,0,30000);
                preferences_need_write = true;
              }
              request->send(200, "text/text", String(blink_interval)); });

    //---------------------------------------------------------------------------
    server.on("/api/hostname", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              if (request->hasParam("set"))
              {
                hostname = request->getParam("set")->value();
                hostname.toLowerCase();
                preferences_need_write = true;
              }
              request->send(200, "text/text", String(hostname)); });

    server.on("/api/deviceinfo", HTTP_GET, [](AsyncWebServerRequest *request)
              {           

                multi_heap_info_t info;
                heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); 

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

                doc["hostname"] = hostname;
                doc["ip"] =  WiFi.localIP().toString();
                doc["mac"] = WiFi.macAddress();
                doc["millis"] = millis();

                doc["ram_free"] = info.total_free_bytes;
                doc["ram_lowest"] = info.minimum_free_bytes;
                doc["ram_largest_free_block"] = info.largest_free_block;
                
                doc["psram_size"] = ESP.getPsramSize();
                doc["psram_free"] = ESP.getFreePsram();
                doc["psram_free_min"] = ESP.getMinFreePsram();
                doc["psram_max_alloc"] = ESP.getMaxAllocPsram();

                doc["fs_used"] = LittleFS.usedBytes();
                doc["fs_total"] = LittleFS.totalBytes();
    
                size_t len = serializeJson(doc, data);
                log_v("Data size: %d", len);
                request->send(200, "text/text", data); });
}

//----------------------------------------------------------------------------------------
//                                                                                 UTILITY
String colorToHex(CRGB col)
{
    char buf[6];
    sprintf(buf, "%02X%02X%02X", col.r, col.g, col.b);
    return String(buf);
}

CRGB hexToColor(String s)
{
    uint32_t res = strtol(s.c_str(), NULL, 16);
    return CRGB(res);
}