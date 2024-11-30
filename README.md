# SetWifi Manager Library

SetWifi Manager adalah library Arduino yang dirancang untuk mempermudah pengelolaan koneksi WiFi pada perangkat ESP32. Library ini menyediakan kemampuan untuk menyimpan kredensial WiFi, beralih ke **Access Point Mode** jika koneksi gagal, serta menawarkan antarmuka web untuk konfigurasi ulang jaringan.

## Fitur
- **Simpan Kredensial**: Menyimpan SSID dan password ke EEPROM.
- **Access Point Mode**: Membuka mode AP untuk konfigurasi ulang jaringan.
- **Web Interface**: Halaman web sederhana untuk memasukkan SSID dan password baru.
- **Auto Reconnect**: Secara otomatis mencoba menyambung kembali ke jaringan WiFi.

## Cara Penggunaan

### 1. Instalasi
1. Unduh atau klon repositori ini.
2. Ekstrak folder `SetWifi` ke direktori library Arduino Anda:


### 2. Contoh Penggunaan
Gunakan file contoh `BasicLedExample.ino` untuk memulai. 
Berikut adalah **starting point** atau template dasar dari library ini. Anda dapat menambahkan lebih banyak kode untuk proyek yang lebih besar 😎.

```cpp
#include <SetWifi.h>

SetWifi setapEsp;

void setup() {
 Serial.begin(115200);
 setapEsp.begin();

 if (setapEsp.isConnected()) {
     Serial.println("Terhubung ke WiFi");
 } else {
     Serial.println("Mode Access Point Aktif");
 }
}

void loop() {
 setapEsp.handle();
}
