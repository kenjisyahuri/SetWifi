#include "SetWifi.h"

SetWifi::SetWifi() : 
    server(nullptr), 
    connected(false), 
    apMode(false), 
    connectStartTime(0) {}

void SetWifi::resetWiFi() {
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_STA);
}

void SetWifi::begin(const char *apName, const char *apIP) {
    // Reset WiFi sebelum memulai
    resetWiFi();

    // Inisialisasi EEPROM
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("Gagal menginisialisasi EEPROM");
        return;
    }

    // Muat kredensial tersimpan
    loadCredentials();

    // Periksa kredensial
    if (ssid.length() == 0 || password.length() == 0) {
        Serial.println("Tidak ada kredensial WiFi tersimpan");
        startAPMode(apName, apIP);
        return;
    }

    // Coba sambungkan ke WiFi
    connectToWiFi();
}

void SetWifi::connectToWiFi() {
    // Debug: Tampilkan detail WiFi
    Serial.println("Debug WiFi Info:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("SSID Length: ");
    Serial.println(ssid.length());
    Serial.print("Password Length: ");
    Serial.println(password.length());

    // Konfigurasi WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    // Debug: Tampilkan MAC Address
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    WiFi.begin(ssid.c_str(), password.c_str());
    
    Serial.println("Mencoba sambung ke WiFi...");
    connectStartTime = millis();
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Percobaan ");
        Serial.print(attempts + 1);
        Serial.println(": Menghubungkan...");
        
        delay(1000);
        
        // Cek status koneksi dan timeout
        if (WiFi.status() == WL_CONNECTED) break;
        
        attempts++;
        if (attempts >= MAX_CONNECT_ATTEMPTS || 
            (millis() - connectStartTime) >= WIFI_CONNECT_TIMEOUT) {
            Serial.println("Gagal terhubung ke WiFi");
            WiFi.disconnect(true);
            startAPMode(DEFAULT_AP_NAME, DEFAULT_AP_IP);
            return;
        }
    }

    // Koneksi berhasil
    connected = true;
    localIP = WiFi.localIP().toString();
    
    Serial.println("\nTerhubung ke WiFi!");
    printNetworkInfo();
    apMode = false;
}

void SetWifi::printNetworkInfo() {
    Serial.println("--- Informasi Jaringan ---");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("IP Address: ");
    Serial.println(localIP);
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet Mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
}

bool SetWifi::isConnected() {
    return connected;
}

String SetWifi::getIPAddress() {
    return localIP;
}

void SetWifi::handle() {
    if (apMode && server) {
        server->handleClient();
        dnsServer.processNextRequest();
    }
}

void SetWifi::startAPMode(const char *apName, const char *apIP) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName);
    
    IPAddress apIPAddr;
    apIPAddr.fromString(apIP);
    WiFi.softAPConfig(apIPAddr, apIPAddr, IPAddress(255, 255, 255, 0));

    dnsServer.start(53, "*", apIPAddr);
    startWebServer();
    
    apMode = true;
    connected = false;
    
    Serial.println("Mode Access Point Aktif");
    Serial.print("SSID: ");
    Serial.println(apName);
    Serial.print("IP Address: ");
    Serial.println(apIP);
}

void SetWifi::startWebServer() {
    server = new WebServer(80);

    server->on("/", [this]() {
        String page = "<!DOCTYPE html>"
                      "<html><body>"
                      "<h1>Konfigurasi WiFi</h1>"
                      "<form method='POST' action='/simpan'>"
                      "SSID: <input type='text' name='ssid'><br>"
                      "Password: <input type='password' name='password'><br>"
                      "<input type='submit' value='Simpan'>"
                      "</form>"
                      "</body></html>";
        server->send(200, "text/html", page);
    });

    server->on("/simpan", [this]() {
        ssid = server->arg("ssid");
        password = server->arg("password");

        saveCredentials();
        
        String responHtml = "<!DOCTYPE html>"
                             "<html><body>"
                             "<h1>Kredensial Tersimpan!</h1>"
                             "<p>Perangkat akan restart...</p>"
                             "<script>setTimeout(function(){window.location.href='/';}, 3000);</script>"
                             "</body></html>";
        
        server->send(200, "text/html", responHtml);
        
        delay(1000);
        ESP.restart();
    });

    server->begin();
}

void SetWifi::saveCredentials() {
    // Bersihkan EEPROM terlebih dahulu
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    
    // Tulis SSID
    EEPROM.writeString(0, ssid);
    // Tulis Password
    EEPROM.writeString(64, password);
    
    EEPROM.commit();
    Serial.println("Kredensial tersimpan di EEPROM");
}

void SetWifi::loadCredentials() {
    ssid = EEPROM.readString(0);
    password = EEPROM.readString(64);
    
    Serial.print("SSID Tersimpan: ");
    Serial.println(ssid);
}