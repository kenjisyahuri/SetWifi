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
                      "<html>"
                      "<head>"
                      "<title>Access Point ESP</title>"
                      "<style>"
                      "body {"
                      "    display: flex;"
                      "    flex-direction: column;"
                      "    align-items: center;"
                      "    padding-top: 10%;"
                      "    height: 100vh;"
                      "    font-family: Arial, sans-serif;"
                      "    background-color: #F5F5F7;"
                      "    margin: 0;"
                      "}"
                      "h1 {"
                      "    font-size: 24px;"
                      "    font-weight: bold;"
                      "    margin-bottom: 5px;"
                      "}"
                      "p {"
                      "    font-size: 16px;"
                      "    opacity: 0.3;"
                      "    margin-bottom: 50px;"
                      "}"
                      ".image-container {"
                      "    width: 150px;"
                      "    height: 150px;"
                      "    margin-bottom: 20px;"
                      "}"
                      ".image-container img {"
                      "    width: 100%;"
                      "    height: 100%;"
                      "}"
                      ".input-container {"
                      "    display: flex;"
                      "    flex-direction: column;"
                      "    align-items: center;"
                      "    width: 100%;"
                      "    max-width: 300px;"
                      "}"
                      ".input-container input {"
                      "    width: 100%;"
                      "    padding: 10px;"
                      "    margin: 10px 0;"
                      "    border: 2px solid #000000;"
                      "    border-radius: 5px;"
                      "    font-size: 16px;"
                      "    text-align: center;"
                      "}"
                      ".input-container button {"
                      "    padding: 10px 20px;"
                      "    border: none;"
                      "    border-radius: 5px;"
                      "    background-color: #B7B7B7;"
                      "    border: 2px solid #000000;"
                      "    font-size: 16px;"
                      "    cursor: pointer;"
                      "}"
                      "</style>"
                      "<script>"
                      "function submitForm() {"
                      "    document.getElementById('ssidHidden').value = document.getElementById('ssidInput').value;"
                      "    document.getElementById('passwordHidden').value = document.getElementById('passwordInput').value;"
                      "    document.getElementById('hiddenForm').submit();"
                      "}"
                      "</script>"
                      "</head>"
                      "<body>"
                      "<h1>Access Point ESP</h1>"
                      "<p>Silakan hubungkan perangkat Anda ke jaringan yang tersedia.</p>"
                      "<div class='image-container'>"
                      "<img alt='ESP module with AP-MODE text and Wi-Fi symbol' height='150' src='src/icon.png' width='150'/>"
                      "</div>"
                      "<div class='input-container'>"
                      "<input id='ssidInput' placeholder='SSID' type='text'/>"
                      "<input id='passwordInput' placeholder='Password' type='password'/>"
                      "<button onclick='submitForm()'>Connect</button>"
                      "</div>"
                      "<form id='hiddenForm' method='POST' action='/simpan' style='display:none;'>"
                      "<input id='ssidHidden' type='hidden' name='ssid' value=''>"
                      "<input id='passwordHidden' type='hidden' name='password' value=''>"
                      "</form>"
                      "</body>"
                      "</html>";
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