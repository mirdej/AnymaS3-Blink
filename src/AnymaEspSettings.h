#ifndef __ANYMA_ESP_SETTINGS_INCLUDED__
#define __ANYMA_ESP_SETTINGS_INCLUDED__

#include "Arduino.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "LittleFS.h"

#define MAIN_FILE_SYSTEM LittleFS
#define SETTINGS_FILE_PATH "/data/settings.json"

#define FTP_USER "me"
#define FTP_PASSWORD "me"

// --------------------------------------------------------------------------------- Global Settings
//                                                      Values can be overwritten
//                                                      Stored in SETTINGS_FILE_PATH on main FS

struct AnymaEspSettings
{
public:
    // -------------------------------------------------------------------------------------------------------------------- CUSTOMIZE THIS PART
    String hostname = "Baby-S3";
    String ssid = "Anymair";
    String pass = "Mot de passe pas complique";
    String blink_color = "00FF00";
    int blink_interval = 1000;
    // -------------------------------------------------------------------------------------------------------------------- END CUSTOM

    //-- storage and retrieval functions
    void begin();
    err_t read();
    err_t write();
    JsonDocument get_json();

    bool dirty;
};

extern AnymaEspSettings settings;

JsonDocument AnymaEspSettings::get_json()
{
    JsonDocument doc;

    // -------------------------------------------------------------------------------------------------------------------- CUSTOMIZE THIS PART
    // Add values in the document
    doc["hostname"] = hostname;
    doc["ssid"] = ssid;
    doc["pass"] = pass;
    doc["blink_color"] = blink_color;
    doc["blink_interval"] = blink_interval;
    // -------------------------------------------------------------------------------------------------------------------- END CUSTOM

    return doc;
}

// --------------------------------------------------------------------------------- READ

err_t AnymaEspSettings::read()
{
    if (!MAIN_FILE_SYSTEM.exists(SETTINGS_FILE_PATH))
    {
        log_e("Settings file does not exist. Create from default values");
        write();
        return 1;
    }
    else
    {

        File file = MAIN_FILE_SYSTEM.open(SETTINGS_FILE_PATH, FILE_READ);
        if (!file)
        {
            log_e("- failed to open file for reading");
            return -2;
        }

        JsonDocument doc;
        deserializeJson(doc, file);

        // -------------------------------------------------------------------------------------------------------------------- CUSTOMIZE THIS PART

        if (doc["hostname"])
            hostname = doc["hostname"].as<String>();
        if (doc["ssid"])
            ssid = doc["ssid"].as<String>();
        if (doc["pass"])
            pass = doc["pass"].as<String>();
        if (doc["blink_color"])
            blink_color = doc["blink_color"].as<String>();
        if (doc["blink_interval"])
            blink_interval = doc["blink_interval"];

        // -------------------------------------------------------------------------------------------------------------------- END CUSTOM

        Serial.println("Settings read:");
        serializeJsonPretty(doc,Serial);

        file.close();
        return 0;
    }
}

// --------------------------------------------------------------------------------- WRITE

err_t AnymaEspSettings::write()
{

    // check for settings file
    if (!MAIN_FILE_SYSTEM.exists("/data"))
    {
        log_i("Creating Directory /data");
        MAIN_FILE_SYSTEM.mkdir("/data");
    }

    File file = MAIN_FILE_SYSTEM.open(SETTINGS_FILE_PATH, FILE_WRITE);

    if (!file)
    {
        log_e("- failed to open file for writing");
        return -2;
    }

    size_t bytes_written = serializeJson(get_json(), file);
    file.close();

    if (bytes_written)
    {
        log_i("Wrote %d bytes to %s", bytes_written, SETTINGS_FILE_PATH);
        dirty = false;
    }
    else
    {
        log_e("Settings write failed");
        return -1;
    }

    return 0;
}

//----------------------------------------------------------------------------------------
//																				                                Preferences Task

void preferences_task(void *)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(30000));
        if (settings.dirty)
        {
            settings.write();
        }
    }
}

void AnymaEspSettings::begin()
{
    read();

    xTaskCreate(
        preferences_task, // Function that implements the task.
        "PREFS Task",     // Text name for the task.
        8192,             // Stack size in words, not bytes.
        NULL,             // Parameter passed into the task.
        0,                // Priority at which the task is created.
        NULL);
}
#endif