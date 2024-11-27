#include <SetWifi.h>

SetWifi wifiManager;
const int pinLED = 2;  // Pin LED pada GPIO2
WiFiServer serverWeb(80);

void setup() {
    // Reset dan konfigurasi awal
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    // Inisialisasi pin LED
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, LOW);

    // Debug: Tampilkan informasi boot
    Serial.println("\nSistem Memulai Konfigurasi...");
    delay(1000);

    // Inisialisasi WiFi
    wifiManager.begin();
    
    // Jika terhubung ke WiFi, mulai server web
    if (wifiManager.isConnected()) {
        serverWeb.begin();
        Serial.println("Server Web Aktif");
        Serial.print("Akses melalui IP: ");
        Serial.println(wifiManager.getIPAddress());
        
        // Berikan indikasi koneksi berhasil
        digitalWrite(pinLED, HIGH);
        delay(500);
        digitalWrite(pinLED, LOW);
    } else {
        // Indikasi mode AP
        for (int i = 0; i < 5; i++) {
            digitalWrite(pinLED, HIGH);
            delay(200);
            digitalWrite(pinLED, LOW);
            delay(200);
        }
    }
}

void loop() {
    // Tangani manajer WiFi
    wifiManager.handle();
    
    // Jika terhubung ke WiFi, tangani klien web
    if (wifiManager.isConnected()) {
        WiFiClient klien = serverWeb.available();
        if (klien) {
            prosesPermintaanWeb(klien);
        }
    }
}

void prosesPermintaanWeb(WiFiClient &klien) {
    String barisSekarang = "";
    String permintaan = "";
    
    while (klien.connected()) {
        if (klien.available()) {
            char c = klien.read();
            permintaan += c;
            
            if (c == '\n') {
                if (barisSekarang.length() == 0) {
                    // Kirim respons HTTP
                    klien.println("HTTP/1.1 200 OK");
                    klien.println("Content-type:text/html");
                    klien.println();
                    
                    // Halaman HTML kontrol LED
                    klien.print(
                        "<!DOCTYPE html>"
                        "<html><body>"
                        "<h1>Kontrol LED</h1>"
                        "<p><a href='/led/on'>Nyalakan LED</a></p>"
                        "</body></html>"
                    );
                    
                    // Periksa perintah kontrol LED
                    if (permintaan.indexOf("GET /led/on") >= 0) {
                        digitalWrite(pinLED, HIGH);
                        Serial.println("LED Dinyalakan");
                    } else if (permintaan.indexOf("GET /led/off") >= 0) {
                        digitalWrite(pinLED, LOW);
                        Serial.println("LED Dimatikan");
                    }
                    
                    break;
                }
                barisSekarang = "";
            } else if (c != '\r') {
                barisSekarang += c;
            }
        }
    }
    
    // Tutup koneksi
    klien.stop();
}