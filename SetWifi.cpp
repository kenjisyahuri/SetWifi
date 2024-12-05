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

    // Endpoint untuk halaman utama
    server->on("/", [this]() {
        String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>AP Mode ESP</title>    
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: flex-start;
            height: 100vh;
            margin: 0;
            background-color: #E4E0E1;
            padding-top: 20px;
        }
        .title {
            display: flex;
            align-items: center;
            justify-content: center;
            margin-bottom: 20px;
        }
        .title .ap {
            font-size: 172px;
            font-weight: bold;
            color: #493628;
            transition: transform 0.6s ease, color 0.6s ease;
            animation: bounce 2s infinite;
        }
        .title .mode-esp {
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            margin-left: 10px;
        }
        .title .mode, .title .esp {
            font-size: 60px;
            font-weight: bold;
            color: #493628;
            text-align: center;
            transition: transform 0.6s ease, color 0.6s ease;
        }
        .title .esp {
            color: #AB886D;
        }
        .separator {
            width: 100%;
            height: 5px;
            background-color: #D6C0B3;
            margin: 5px 0;
            transition: transform 0.6s ease, background-color 0.6s ease;
        }
        .title:hover .ap {
            transform: scale(1.2) rotate(-5deg);
            color: #AB886D;
        }
        .title:hover .mode {
            transform: translateX(10px);
            color: #D6C0B3;
        }
        .title:hover .esp {
            transform: translateY(-10px) scale(1.1);
            color: #493628;
            text-shadow: 0 5px 15px rgba(73, 54, 40, 0.5);
        }
        .title:hover .separator {
            transform-origin: left center; 
            transform: rotate(90deg) translate(-90px, -50%);
            background-color: #AB886D;
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.2);
            transition: transform 0.6s ease, background-color 0.6s ease;
        }
        @keyframes bounce {
            0%, 100% {
                transform: translateY(0);
            }
            50% {
                transform: translateY(-15px);
            }
        }
        .subtitle {
            font-size: 30px;
            color: #493628;
            margin-bottom: 40px;
            text-align: center;
        }
        .form-group {
            display: flex;
            flex-direction: column;
            align-items: center;
            margin-bottom: 20px;
            width: 100%;
            max-width: 400px;
        }
        .form-group input {
            width: 100%;
            padding: 25px;
            margin: 10px 0;
            border: 4px solid #493628;
            border-radius: 5px;
            font-size: 35px;
            text-align: center;
            background-color: #D6C0B3;
            color: #493628;
        }
        .form-group input::placeholder {
            color: #493628;
        }
        .form-group input:focus {
            outline: none;
            border-color: #AB886D;
            background-color: #E4E0E1;
        }
        .connect-btn {
            padding: 20px 40px;
            border: none;
            border-radius: 8px;
            background-color: #AB886D;
            border: 4px solid #493628;
            font-size: 35px;
            cursor: pointer;
            color: #E4E0E1;
            transition: background-color 0.3s ease, transform 0.3s ease;
        }
        .connect-btn:hover {
            background-color: #493628;
            color: #D6C0B3;
            transform: translateY(-2px);
        }
    </style>
</head>
<body>
    <div class="title">
        <div class="ap">AP</div>
        <div class="mode-esp">
            <div class="mode">MODE</div>
            <div class="separator"></div>
            <div class="esp">ESP</div>
        </div>
    </div>
    <div class="subtitle">Silakan hubungkan perangkat Anda ke jaringan yang tersedia!</div>
    <div class="form-group">
        <input type="text" id="ssid" name="ssid" placeholder="SSID">
        <input type="password" id="password" name="password" placeholder="Password">
    </div>
    <button class="connect-btn" onclick="submitForm()">Connect</button>
    <form id="hiddenForm" method="POST" action="/simpan" style="display:none;">
        <input type="hidden" id="ssidHidden" name="ssid">
        <input type="hidden" id="passwordHidden" name="password">
    </form>
    <script>
        function submitForm() {
            document.getElementById('ssidHidden').value = document.getElementById('ssid').value;
            document.getElementById('passwordHidden').value = document.getElementById('password').value;
            document.getElementById('hiddenForm').submit();
        }
    </script>
</body>
</html>
        )rawliteral";

        server->send(200, "text/html", page);
    });

    // Endpoint untuk menyimpan kredensial
    server->on("/simpan", [this]() {
        ssid = server->arg("ssid");
        password = server->arg("password");

        saveCredentials();

        String responseHtml = "<!DOCTYPE html>"
                              "<html><body>"
                              "<h1>Kredensial Tersimpan!</h1>"
                              "<p>Perangkat akan restart...</p>"
                              "<script>setTimeout(function(){window.location.href='/';}, 3000);</script>"
                              "</body></html>";

        server->send(200, "text/html", responseHtml);

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