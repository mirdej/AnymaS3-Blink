#ifndef __ANYMA_ESP_NETWWORKING_INCLUDED__
#define __ANYMA_ESP_NETWWORKING_INCLUDED__

#include <Arduino.h>
#include <Preferences.h>

#include "FS.h"
#include <LittleFS.h>

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <aWOT.h>
#include "MimeTypes.h"
#include <ESPmDNS.h>

#include "AnymaEspSettings.h"
#include "AnymaEspAPI.h"

#include "ESP-FTP-Server-Lib.h"
#include "FTPFilesystem.h"

#define WEBROOT "/webroot"
#define READ_BUFFER_SIZE 512
const char *NTP_SERVER = "pool.ntp.org";
const char *TZ_STRING = "CET-1CEST,M3.5.0/2,M10.5.0/3";

WiFiMulti wifiMulti;
FTPServer ftp;
WiFiServer server(80);
Application app;

struct AnymaEspNetworking
{
public:
    void begin();
    int num_client_connections = 0;

private:
};

extern AnymaEspNetworking networking;

const char *indexFile = "/index.html";

void fileServer(Request &req, Response &res)
{

    if (req.method() != Request::GET)
    {
        return;
    }



    const char *path = req.path();

    if (strcmp(path, "/") == 0)
    {
        path = indexFile;
    }


    char local_path[120];
    strcpy(local_path, WEBROOT);
    strcat(local_path, path);

    log_v("Request file %s -> %s", path,local_path);

    if (!MAIN_FILE_SYSTEM.exists(local_path))
    {
        return;
    }

    File file = MAIN_FILE_SYSTEM.open(local_path);

    if (file.isDirectory())
    {
        file.close();
        return;
    }

    uint8_t readBuffer[READ_BUFFER_SIZE];
    const char *mimeType = MimeTypes::getType(file.name());
    res.set("Content-Type", mimeType);

    size_t length = file.size();

    while (length)
    {

        size_t toRead = length > READ_BUFFER_SIZE ? READ_BUFFER_SIZE : length;
        file.read(readBuffer, toRead);
        res.write(readBuffer, toRead);
        length = length - toRead;
        taskYIELD();
    }
    file.close();
    res.end();
}

//----------------------------------------------------------------------------------------
//																				                                       WiFi Task
void TaskClientSocket(void *pvParameters)
{
    WiFiClient clientHandle = *((WiFiClient *)pvParameters);
    int client_id = networking.num_client_connections++;

    if (clientHandle.connected())
    {
        log_v("New Client Connected. Client#: %d", client_id);
        app.process(&clientHandle);
        clientHandle.stop();
    }
    log_v("Client #%d disconnected", client_id);
    networking.num_client_connections--;
    vTaskDelete(NULL);
}

void wifi_task(void *)
{
    configTime(0, 0, NTP_SERVER);
    setenv("TZ", TZ_STRING, 1);
    tzset();

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(settings.hostname.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // better resonsiveness, more power consumption
    wifiMulti.addAP(settings.ssid.c_str(), settings.pass.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        wifiMulti.run();
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    log_i("IP address: %s", WiFi.localIP().toString().c_str());

    if (!MDNS.begin(settings.hostname.c_str()))
    {
        log_e("Error setting up MDNS responder!");
    }
    else
    {
        log_i("mDNS responder started");
        log_i("Hostname: %s", settings.hostname);
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ftp", "tcp", 21);
        MDNS.addService("anyma_esp32", "tcp", 86291);
    }

    // setup_webserver();
    
    setup_api();

    app.get(&fileServer);

    server.begin();

    log_i("Webserver started.");

    ftp.addUser(FTP_USER, FTP_PASSWORD);
    ftp.addFilesystem("LittleFS", &LittleFS);
    ftp.begin();
    log_i("FTP server started.");

    while (1)
    {

        if (WiFi.status() == WL_CONNECTED)
        {
            ftp.handle();

            WiFiClient client = server.available();
            if (client)
            {
                xTaskCreatePinnedToCore(
                    TaskClientSocket, "TaskClientSocket", 8192, &client, 2, NULL, 1);
            }
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

void AnymaEspNetworking::begin()
{
    xTaskCreate(
        wifi_task,    // Function that implements the task.
        "WIFI-Setup", // Text name for the task.
        8192,         // Stack size in words, not bytes.
        NULL,         // Parameter passed into the task.
        0,            // Priority at which the task is created.
        NULL);
}
#endif