#include <stdint.h>
#include <LCD.h>
#include <Touch.h>
#include <XPT2046.h>
#include <SPI.h>
#include <TimeLib.h>

#define __AVR_ATmega32U4__

// Global variables for demonstration
float currentTemp = 25.5;
float maxTemp = 30.5;
float minTemp = 20.3;

// Adjustment amount
#define TEMP_ADJUST 0.5

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

void setup() {
    // Start Serial for time sync
    Serial.begin(9600);

    // Initialize SPI
    SPI.setDataMode(SPI_MODE3);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    SPI.begin();

    // Initialize LCD and Touch
    Tft.lcd_init();
    Tft.setRotation(Rotation_0_D);
    Tp.tp_init();

    // Set time to compile time
    setTime(__TIME__, __DATE__);

    // Draw initial display
    drawDisplay();
}

void loop() {
    static uint8_t lastMinute = 0;
    uint8_t currentMinute = minute(now());

    // Check for touch input
    if (Tp.tp_scan(1)) {
        uint16_t rawX = Tp.s_tTouch.hwXpos;
        uint16_t rawY = Tp.s_tTouch.hwYpos;

        // Invert and adjust mapping
        uint16_t x = map(rawX, TOUCH_X_MAX, TOUCH_X_MIN, 0, SCREEN_WIDTH);  // Inverted X mapping
        uint16_t y = map(rawY, TOUCH_Y_MAX, TOUCH_Y_MIN, 0, SCREEN_HEIGHT); // Inverted Y mapping

        Serial.print("Raw X: ");
        Serial.print(rawX);
        Serial.print(" Raw Y: ");
        Serial.print(rawY);
        Serial.print(" Mapped X: ");
        Serial.print(x);
        Serial.print(" Mapped Y: ");
        Serial.println(y);

        // Draw touch point for debugging
        Tft.lcd_draw_point(x, y, WHITE);
        
        handleButtonPress(x, y);
    }

    // Update time if needed
    if (currentMinute != lastMinute) {
        lastMinute = currentMinute;
        drawDateTime();
    }
}

// Function to update only the temperature value display
void updateTempValue(float value, uint16_t x, uint16_t y, uint16_t color) {
    // Clear the previous value area
    Tft.lcd_fill_rect(x + 30, y, 70, 20, BLACK);
    
    // Display new value
    char tempStr[10];
    dtostrf(value, 5, 1, tempStr);
    strcat(tempStr, " C");
    Tft.lcd_display_string(x + 30, y, (const uint8_t*)tempStr, FONT_1608, color);
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
    }

    // MAX Increase button
    else if (x >= BUTTON_MAX_INC_X && x <= (BUTTON_MAX_INC_X + BUTTON_WIDTH) &&    
        y >= BUTTON_MAX_Y && y <= (BUTTON_MAX_Y + BUTTON_HEIGHT)) {    
        Serial.println("MAX increase pressed!");
        maxTemp += TEMP_ADJUST;
        updateTempValue(maxTemp, MAX_VALUE_X, MAX_VALUE_Y, RED);
    }

    // MIN Decrease button
    else if (x >= BUTTON_MIN_DEC_X && x <= (BUTTON_MIN_DEC_X + BUTTON_WIDTH) &&      
        y >= BUTTON_MIN_Y && y <= (BUTTON_MIN_Y + BUTTON_HEIGHT)) {    
        Serial.println("MIN decrease pressed!");
        minTemp -= TEMP_ADJUST;
        updateTempValue(minTemp, MIN_VALUE_X, MIN_VALUE_Y, BLUE);
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
    }
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
