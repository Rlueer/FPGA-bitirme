#include <stdint.h>
#include <LCD.h>
#include <Touch.h>
#include <XPT2046.h>
#include <SPI.h>
#include <TimeLib.h>
#include <Wire.h>

#define __AVR_ATmega32U4__

// Global variables for demonstration
int currentTemp = 25; 
int maxTemp = 30;
int minTemp = 20;
uint8_t counter = 1; // Sayacımız veri gönderim için kullanılıyor
bool sendMode = true; // Gönderim ve alım modunu belirten bayrak
unsigned long lastMillis = 0; // Son zaman kaydı
const unsigned long interval = 1000; // 5 saniyelik periyot
const uint8_t FPGA_ADDR = 0x42; // FPGA'nın I2C adresi
int errorCount = 0; // Hata sayısını takip etmek için sayaç
const int maxErrors = 3; // Maksimum hata sayısı

// Flags to track changes in thresholds
bool maxTempChanged = false;
bool minTempChanged = false;


// Adjustment amount
#define TEMP_ADJUST 1

// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Raw touch calibration values
#define TOUCH_X_MIN 230   // Min X value (down-right corner)
#define TOUCH_X_MAX 1862  // Max X value (down-left corner)
#define TOUCH_Y_MIN 131   // Min Y value (down-left corner)
#define TOUCH_Y_MAX 1873  // Max Y value (top-left corner)

#define BUTTON_WIDTH 40            // Halved width
#define BUTTON_HEIGHT 30           // Slightly reduced height

// MAX buttons (adjacent)
#define BUTTON_MAX_DEC_X 120       // Decrease MAX button
#define BUTTON_MAX_INC_X 165       // Increase MAX button - right next to decrease
#define BUTTON_MAX_Y 160           // Y position for MAX buttons

// MIN buttons (adjacent)
#define BUTTON_MIN_DEC_X 120       // Decrease MIN button
#define BUTTON_MIN_INC_X 165       // Increase MIN button - right next to decrease
#define BUTTON_MIN_Y 220           // Y position for MIN buttons

// Text positions
#define TEMP_DISPLAY_Y 120
#define MAX_TEXT_Y 160
#define MIN_TEXT_Y 220

// Value display positions
#define MAX_VALUE_X 20
#define MAX_VALUE_Y 160
#define MIN_VALUE_X 20
#define MIN_VALUE_Y 220

// Weekday names
const char* dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void updateTempValueDisplay();
void sendTempToFPGA();
void resetArduino();


void resetArduino() {
    Serial.println("Resetting Arduino due to repeated errors...");
    Wire.end(); // I2C bağlantısını sonlandır
    Wire.begin(); // I2C'yi yeniden başlat
    Wire.setClock(50000); // I2C hızı yeniden ayarlanıyor
    errorCount = 0; // Hata sayacını sıfırla
    counter = 1; // Sayaç sıfırlanıyor
    sendMode = true; // Gönderim moduna geç
    lastMillis = millis(); // Zamanı güncelle
}

void recoverI2CBus() {
    Serial.println("Recovering I2C bus...");
    pinMode(SCL, OUTPUT);
    pinMode(SDA, OUTPUT);
    for (int i = 0; i < 9; i++) { // Generate 9 clock pulses
        digitalWrite(SCL, HIGH);
        delayMicroseconds(10);
        digitalWrite(SCL, LOW);
        delayMicroseconds(10);
    }
    digitalWrite(SCL, HIGH);
    digitalWrite(SDA, HIGH); // Release SDA
    delayMicroseconds(10);
    Wire.begin(); // Reinitialize I2C
    Wire.setClock(50000);
    Serial.println("I2C bus recovered.");
}

void setup() {
    // Start Serial for I2C and debugging
    Serial.begin(9600);
    // Initialize I2C
    Wire.begin(); // I2C başlangıç fonksiyonu
    Wire.setClock(50000);  // I2C hızı 50kHz olarak ayarlanıyor

    // Initialize SPI
    SPI.setDataMode(SPI_MODE3);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    SPI.begin();

    // Initialize LCD and Touch
    Tft.lcd_init();
    Tft.setRotation(Rotation_0_D);
    Tp.tp_init();

    // Set initial display
    drawDisplay();

    delay(1000); // Allow system components to stabilize
}

void loop() {
    static uint8_t lastMinute = 0;
    uint8_t currentMinute = minute(now());
    unsigned long currentMillis = millis();
    int to_fpga=0;
    // Handle I2C communication every 5 seconds
    if (currentMillis - lastMillis >= interval) {
        
        lastMillis = currentMillis;
        
        if (sendMode ) {
            // If thresholds changed, send them to FPGA
            if (maxTempChanged) {
              to_fpga=maxTemp;
            }
            else if (minTempChanged) {
              to_fpga=minTemp;
            }

          // Send regular counter value
          Wire.beginTransmission(FPGA_ADDR);
          Wire.write(to_fpga);
          byte error = Wire.endTransmission();

          if (error == 0) {
              Serial.print("Data sent successfully: ");
              Serial.println(to_fpga);
              maxTempChanged=false;
              minTempChanged=false;
              errorCount = 0;
          } else {
              Serial.print("Error sending data: ");
              Serial.println(error);
              errorCount++;
          }
        } else {  
            // Receive current temperature from FPGA
            byte receivedBytes = Wire.requestFrom((uint8_t)FPGA_ADDR, (uint8_t)1);
            if (receivedBytes > 0 && Wire.available()) {
                byte data = Wire.read(); // Receive the 8-bit value
                currentTemp = data+5;
                updateCurrentTempDisplay();
                Serial.print("Received (dec): ");
                Serial.println(data+5, DEC);
                errorCount = 0;
            } else {
                Serial.println("No response from FPGA!");
                errorCount++;
            }
        }

        if (errorCount >= maxErrors) {
            resetArduino();
        }

        sendMode = !sendMode; // Switch mod
    }

        // Check for touchscreen input
      if (Tp.tp_scan(1)) {
          uint16_t rawX = Tp.s_tTouch.hwXpos;
          uint16_t rawY = Tp.s_tTouch.hwYpos;

          uint16_t x = map(rawX, TOUCH_X_MAX, TOUCH_X_MIN, 0, SCREEN_WIDTH);
          uint16_t y = map(rawY, TOUCH_Y_MAX, TOUCH_Y_MIN, 0, SCREEN_HEIGHT);
          Tft.lcd_draw_point(x, y, WHITE);

          handleButtonPress(x, y);
      }

      // Update the time on the display
      if (currentMinute != lastMinute) {
          lastMinute = currentMinute;
          drawDateTime();
      }
}

// Function to update only the temperature value display
void updateTempValue(int value, uint16_t x, uint16_t y, uint16_t color) {
    // Clear the previous value area
    Tft.lcd_fill_rect(x + 30, y, 70, 20, BLACK);
    
    // Display new value
    char tempStr[10];
    dtostrf(value, 5, 1, tempStr);
    strcat(tempStr, " C");
    Tft.lcd_display_string(x + 30, y, (const uint8_t*)tempStr, FONT_1608, color);
}


void updateCurrentTempDisplay() {
    // Clear the area displaying the current temperature
    Tft.lcd_fill_rect(60, 120, 80, 20, BLACK);

    // Display the new current temperature
    char tempStr[10];
    sprintf(tempStr, "%d C", currentTemp);
    Tft.lcd_display_string(60, 120, (const uint8_t*)tempStr, FONT_1608, BLUE);
}

void handleButtonPress(uint16_t x, uint16_t y) {
    bool valueChanged = false;
    
    // MAX Decrease button
    if (x >= BUTTON_MAX_DEC_X && x <= (BUTTON_MAX_DEC_X + BUTTON_WIDTH) &&      
        y >= BUTTON_MAX_Y && y <= (BUTTON_MAX_Y + BUTTON_HEIGHT)) {    
        Serial.println("MAX decrease pressed!");
        maxTemp -= TEMP_ADJUST;
        if (maxTemp <= minTemp) {
            maxTemp = minTemp + TEMP_ADJUST;
        }
        updateTempValue(maxTemp, MAX_VALUE_X, MAX_VALUE_Y, RED);
        maxTempChanged=true;
    }

    // MAX Increase button
    else if (x >= BUTTON_MAX_INC_X && x <= (BUTTON_MAX_INC_X + BUTTON_WIDTH) &&    
        y >= BUTTON_MAX_Y && y <= (BUTTON_MAX_Y + BUTTON_HEIGHT)) {    
        Serial.println("MAX increase pressed!");
        maxTemp += TEMP_ADJUST;
        updateTempValue(maxTemp, MAX_VALUE_X, MAX_VALUE_Y, RED);
        maxTempChanged=true;
    }

    // MIN Decrease button
    else if (x >= BUTTON_MIN_DEC_X && x <= (BUTTON_MIN_DEC_X + BUTTON_WIDTH) &&      
        y >= BUTTON_MIN_Y && y <= (BUTTON_MIN_Y + BUTTON_HEIGHT)) {    
        Serial.println("MIN decrease pressed!");
        minTemp -= TEMP_ADJUST;
        updateTempValue(minTemp, MIN_VALUE_X, MIN_VALUE_Y, BLUE);
        minTempChanged=true;
    }

    // MIN Increase button
    else if (x >= BUTTON_MIN_INC_X && x <= (BUTTON_MIN_INC_X + BUTTON_WIDTH) &&    
        y >= BUTTON_MIN_Y && y <= (BUTTON_MIN_Y + BUTTON_HEIGHT)) {    
        Serial.println("MIN increase pressed!");
        minTemp += TEMP_ADJUST;
        if (minTemp >= maxTemp) {
            minTemp = maxTemp - TEMP_ADJUST;
        }
        updateTempValue(minTemp, MIN_VALUE_X, MIN_VALUE_Y, BLUE);
        minTempChanged=true;
    }
    delay(20);
} 

void drawDisplay() {
    // Clear screen with black background
    Tft.lcd_clear_screen(BLACK);

    // Draw date and time
    drawDateTime();

    // Display "TEMPERATURE" in green
    Tft.lcd_display_string(40, 80, (const uint8_t*)"TEMPERATURE", FONT_1608, GREEN);

    // Display current temperature in blue
    char tempStr[10];
    dtostrf(currentTemp, 5, 1, tempStr);
    strcat(tempStr, " C");
    Tft.lcd_display_string(60, 120, (const uint8_t*)tempStr, FONT_1608, BLUE);

    // Display MAX label and value
    Tft.lcd_display_string(MAX_VALUE_X, MAX_VALUE_Y, (const uint8_t*)"MAX:", FONT_1608, RED);
    updateTempValue(maxTemp, MAX_VALUE_X, MAX_VALUE_Y, RED);

    // Draw MAX buttons - smaller and adjacent
    Tft.lcd_fill_rect(BUTTON_MAX_DEC_X, BUTTON_MAX_Y, BUTTON_WIDTH, BUTTON_HEIGHT, RED);
    Tft.lcd_display_string(BUTTON_MAX_DEC_X + 15, BUTTON_MAX_Y + 6, (const uint8_t*)"-", FONT_1608, WHITE);

    Tft.lcd_fill_rect(BUTTON_MAX_INC_X, BUTTON_MAX_Y, BUTTON_WIDTH, BUTTON_HEIGHT, RED);
    Tft.lcd_display_string(BUTTON_MAX_INC_X + 15, BUTTON_MAX_Y + 6, (const uint8_t*)"+", FONT_1608, WHITE);

    // Display MIN label and value
    Tft.lcd_display_string(MIN_VALUE_X, MIN_VALUE_Y, (const uint8_t*)"MIN:", FONT_1608, BLUE);
    updateTempValue(minTemp, MIN_VALUE_X, MIN_VALUE_Y, BLUE);

    // Draw MIN buttons - smaller and adjacent
    Tft.lcd_fill_rect(BUTTON_MIN_DEC_X, BUTTON_MIN_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BLUE);
    Tft.lcd_display_string(BUTTON_MIN_DEC_X + 15, BUTTON_MIN_Y + 6, (const uint8_t*)"-", FONT_1608, WHITE);

    Tft.lcd_fill_rect(BUTTON_MIN_INC_X, BUTTON_MIN_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BLUE);
    Tft.lcd_display_string(BUTTON_MIN_INC_X + 15, BUTTON_MIN_Y + 6, (const uint8_t*)"+", FONT_1608, WHITE);
}

void drawDateTime() {
    // Clear just the date/time area
    Tft.lcd_fill_rect(20, 20, 200, 40, BLACK);

    // Format and display date
    char dateStr[30];
    sprintf(dateStr, "%s, %d/%d/%d", 
            dayNames[weekday(now()) - 1],
            day(now()),
            month(now()),
            year(now()));
    Tft.lcd_display_string(20, 20, (const uint8_t*)dateStr, FONT_1608, GREEN);

    // Format and display time (hours and minutes only)
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", hour(now()), minute(now()));
    Tft.lcd_display_string(20, 40, (const uint8_t*)timeStr, FONT_1608, BLUE);
}

void setTime(const char* timeStr, const char* dateStr) {
    int h, m, s;
    sscanf(timeStr, "%d:%d:%d", &h, &m, &s);

    char monthStr[4];
    int d, y;
    sscanf(dateStr, "%s %d %d", monthStr, &d, &y);

    int month = 1;
    const char* monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) {
        if (strncmp(monthStr, monthNames[i], 3) == 0) {
            month = i + 1;
            break;
        }
    }

    setTime(h, m, s, d, month, y);
} 