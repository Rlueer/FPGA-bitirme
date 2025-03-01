#include <Wire.h>

uint8_t counter = 1; // Sayacımız veri gönderim için kullanılıyor
bool sendMode = true; // Gönderim ve alım modunu belirten bayrak
unsigned long lastMillis = 0; // Son zaman kaydı
const unsigned long interval = 5000; // 5 saniyelik periyot
const uint8_t FPGA_ADDR = 0x42; // FPGA'nın I2C adresi
int errorCount = 0; // Hata sayısını takip etmek için sayaç
const int maxErrors = 3; // Maksimum hata sayısı

void resetArduino() {
    Serial.println("Resetting Arduino due to repeated errors...");
    Wire.end(); // I2C bağlantısını sonlandır
    delay(100); // Kısa bir bekleme süresi
    Wire.begin(); // I2C'yi yeniden başlat
    Wire.setClock(50000); // I2C hızı yeniden ayarlanıyor
    errorCount = 0; // Hata sayacını sıfırla
    counter = 1; // Sayaç sıfırlanıyor
    sendMode = true; // Gönderim moduna geç
    lastMillis = millis(); // Zamanı güncelle
    delay(1000); // Sistem stabilizasyonu için bekle
    Serial.println("Arduino reset completed.");
}

void setup() {
    Serial.begin(9600); // Seri haberleşme başlatılıyor
    Wire.begin(); // I2C başlangıç fonksiyonu
    Wire.setClock(50000);  // I2C hızı 50kHz olarak ayarlanıyor
    delay(1000);  // Haberleşme için 1 saniye bekleniyor
}

void loop() {
    unsigned long currentMillis = millis(); // Geçerli zamanı al

    if (currentMillis - lastMillis >= interval) { // 5 saniye geçtiyse
        lastMillis = currentMillis; // Zaman kaydını güncelle

        if (sendMode) { // Gönderim modu
            Wire.beginTransmission(FPGA_ADDR); // FPGA'ya I2C aktarımı başlat
            Wire.write(counter); // Sayaç verisini gönder
            byte error = Wire.endTransmission(); // Aktarımı bitir ve hata kodunu al

            if (error == 0) { // Hata yoksa
                Serial.print("Data sent successfully: ");
                Serial.println(counter); // Gönderilen veriyi yazdır
                counter++; // Sayaç artır
                errorCount = 0; // Hata sayacını sıfırla
            } else { // Hata varsa
                Serial.print("Error sending data: ");
                Serial.println(error); // Hata kodunu yazdır
                errorCount++; // Hata sayısını artır
            }
        } else { // Alım modu
            byte receivedBytes = Wire.requestFrom((uint8_t)FPGA_ADDR, (uint8_t)1); // FPGA'dan 1 byte veri talep et
            if (receivedBytes > 0 && Wire.available()) { // Veri geldiyse
                byte data = Wire.read(); // Gelen veriyi oku
                Serial.print("Received (dec): ");
                Serial.println(data, DEC); // Veriyi yazdır
                errorCount = 0; // Hata sayacını sıfırla
            } else { // Veri gelmediyse
                Serial.println("No response from FPGA! Devam ediyor..."); // FPGA'dan cevap yok
                errorCount++; // Hata sayısını artır
            }
        }

        if (errorCount >= maxErrors) {
            resetArduino(); // Maksimum hata sayısına ulaşıldığında Arduino'yu resetle
        }

        sendMode = !sendMode; // Modu değiştir (gönderim/alım)
    }
}
