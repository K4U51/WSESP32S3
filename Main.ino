#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "CST820.h"

#include "Speedo_demo.h"   // Your LVGL layout from SquareLine
#include "Wireless.h"
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include "LVGL_Driver.h"
#include "BAT_Driver.h"

// ---------- Global Variables ----------
float ax = 0, ay = 0, az = 0;
FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from RTC_PCF85063.h

// LVGL objects
lv_obj_t *gball_dot = NULL;
const int centerX = 120;  // Center of the dial (adjust for your layout)
const int centerY = 120;
const int radius  = 80;   // Radius of G-circle in pixels

// ---------- Function Prototypes ----------
void Build_UI(void);
void getAccelerometerData(void);
void updateForceGauge(float x, float y);
void create_gball_dot(void);
void update_gball_dot(float x, float y);

// ---------- Accelerometer Reading ----------
void getAccelerometerData() {
    getAccelerometer(); // Reads accelerometer in Gs from QMI8658
    ax = Accel.x;
    ay = Accel.y;
    az = Accel.z;
}

// ---------- LVGL UI Setup ----------
void Build_UI(void) {
    ui_init();       // Loads your SquareLine LVGL layout
    make_dial();     // Custom dial background, if defined
    create_gball_dot();  // Create dot object on top of dial
}

// ---------- Create Dot ----------
void create_gball_dot(void) {
    gball_dot = lv_obj_create(lv_scr_act());
    lv_obj_set_size(gball_dot, 14, 14);
    lv_obj_set_style_radius(gball_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(gball_dot, lv_color_hex(0xFF0000), 0); // Red dot
    lv_obj_clear_flag(gball_dot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(gball_dot, centerX - 7, centerY - 7); // center it
}

// ---------- Update Dot Position ----------
void update_gball_dot(float x, float y) {
    // Clamp values to ±2G
    if (x > 2.0f) x = 2.0f;
    if (x < -2.0f) x = -2.0f;
    if (y > 2.0f) y = 2.0f;
    if (y < -2.0f) y = -2.0f;

    // Map ±2G → ±radius pixels
    int posX = centerX + (int)((x / 2.0f) * radius);
    int posY = centerY - (int)((y / 2.0f) * radius); // invert Y axis for display

    // Move the dot
    lv_obj_set_pos(gball_dot, posX - 7, posY - 7);
}

// ---------- G-Force Visualization ----------
void updateForceGauge(float x, float y) {
    // Update moving dot position
    update_gball_dot(x, y);

    // Optionally calculate and print total G
    float gForce = sqrtf(x * x + y * y);
    if (gForce > 2.0f) gForce = 2.0f;

    // Debug output
    printf("X: %.2f  Y: %.2f  Total G: %.2f\n", x, y, gForce);
}

// ---------- Main Application ----------
void app_main(void)
{
    // ---------- Hardware Initialization ----------
    Wireless_Init();
    Flash_Searching();
    I2C_Init();
    PCF85063_Init();
    QMI8658_Init();
    EXIO_Init();
    LCD_Init();
    Touch_Init();
    SD_Init();
    LVGL_Init();

    // ---------- Initialize UI ----------
    Build_UI();

    // ---------- Open SD Log ----------
    logFile = fopen("/sdcard/gforce_log.csv", "w");
    if (logFile) {
        fprintf(logFile, "Timestamp,Ax,Ay,Az\n");
        fflush(logFile);
        printf("✅ Logging started: /sdcard/gforce_log.csv\n");
    } else {
        printf("⚠️ SD log file could not be opened.\n");
    }

    // ---------- Main Loop ----------
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50)); // ~20Hz refresh rate

        lv_timer_handler();             // Update LVGL UI
        PCF85063_Read_Time(&datetime);
        getAccelerometerData();
        updateForceGauge(ax, ay);

        // ---------- Log to SD ----------
        if (logFile) {
            fprintf(logFile, "%02d:%02d:%02d,%.3f,%.3f,%.3f\n",
                    datetime.hour, datetime.minute, datetime.second,
                    ax, ay, az);
            fflush(logFile);
        }
    }

    // ---------- Close file (never reached normally) ----------
    if (logFile) fclose(logFile);
}
