#ifndef SETWIFI_H
#define SETWIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

#define DEFAULT_AP_NAME "Portal_Captive_ESP"
#define DEFAULT_AP_IP "192.168.4.1"
#define EEPROM_SIZE 512
#define WIFI_CONNECT_TIMEOUT 20000  // Waktu timeout koneksi WiFi (20 detik)
#define MAX_CONNECT_ATTEMPTS 10

class SetWifi {
public:
    SetWifi();
    void begin(const char *apName = DEFAULT_AP_NAME, const char *apIP = DEFAULT_AP_IP);
    bool isConnected();
    String getIPAddress();
    void handle();
    void printNetworkInfo();

private:
    void startAPMode(const char *apName, const char *apIP);
    void startWebServer();
    void saveCredentials();
    void loadCredentials();
    void connectToWiFi();
    void resetWiFi();

    WebServer *server;
    DNSServer dnsServer;
    String ssid;
    String password;
    String localIP;
    bool connected;
    bool apMode;
    unsigned long connectStartTime;
};

#endif