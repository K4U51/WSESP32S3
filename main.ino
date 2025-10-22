#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "CST820.h"

#include "Wireless.h"
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include "LVGL_Driver.h"
#include "BAT_Driver.h"
#include "ui.h"           // SquareLine Studio UI
#include "GForceUI.h"     // Modular G-Force handling
#include "ui_hook.h"      // Connect LVGL objects to GForceUI

FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from PCF85063 RTC

#define UPDATE_RATE_MS 50
#define SMOOTH_FACTOR 0.2
#define G_MAX 2.5f    // Max G for mapping gauges

float smoothed_ax = 0;
float smoothed_ay = 0;
float smoothed_az = 0;

// ---------- Simple linear interpolation ----------
static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Map float value to gauge integer range
static int mapFloatToGauge(float value, float min_val, float max_val, int gauge_min, int gauge_max) {
    if (value < min_val) value = min_val;
    if (value > max_val) value = max_val;
    return (int)((value - min_val) * (gauge_max - gauge_min) / (max_val - min_val) + gauge_min);
}

// ---------- Generate unique SD log filename ----------
static void generate_log_filename(char *filename, int max_len) {
    PCF85063_Read_Time(&datetime);
    int session = 1;
    do {
        snprintf(filename, max_len,
                 "/sdcard/gforce_%04d%02d%02d_%02d%02d%02d_%d.csv",
                 datetime.year, datetime.month, datetime.day,
                 datetime.hour, datetime.minute, datetime.second,
                 session);
        FILE *file = fopen(filename, "r");
        if (file) { fclose(file); session++; if(session > 999) break; }
        else break;
    } while(1);
}

void app_main(void) {
    printf("🚀 Starting G-Force UI with SquareLine...\n");

    // ---------- Initialize Hardware ----------
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
    ui_init();
    hook_gforce_ui();   // Assign LVGL objects to GForceUI
    printf("✅ UI loaded and hooks applied.\n");

    // Set initial dot position if available
    if (ui_dot) lv_obj_set_pos(ui_dot, 240, 240);

    // ---------- Generate SD log filename ----------
    char filename[128];
    generate_log_filename(filename, sizeof(filename));

    logFile = fopen(filename, "w");
    if (logFile) {
        fprintf(logFile, "Timestamp,Ax,Ay,Az,PeakAccel,PeakBrake,PeakLeft,PeakRight\n");
        fflush(logFile);
        printf("✅ Logging started: %s\n", filename);
    } else {
        printf("⚠️ Could not open SD log file: %s\n", filename);
    }

    // ---------- Main Loop ----------
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(UPDATE_RATE_MS));

        lv_timer_handler();        // Refresh LVGL

        // Read accelerometer
        getAccelerometerData();

        // Smooth accelerometer
        smoothed_ax = smoothed_ax * (1.0 - SMOOTH_FACTOR) + ax * SMOOTH_FACTOR;
        smoothed_ay = smoothed_ay * (1.0 - SMOOTH_FACTOR) + ay * SMOOTH_FACTOR;
        smoothed_az = smoothed_az * (1.0 - SMOOTH_FACTOR) + az * SMOOTH_FACTOR;

        // Update GForceUI
        update_gforce_ui(smoothed_ax, smoothed_ay, smoothed_az);

        // Optional: Update primary gauge from SquareLine
        if (ui_gauge_accel) lv_gauge_set_value(ui_gauge_accel, 0, mapFloatToGauge(smoothed_ay, -G_MAX, G_MAX, 0, 100));

        // Log to SD
        if (logFile) {
            fprintf(logFile, "%04d-%02d-%02d %02d:%02d:%02d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                    datetime.year, datetime.month, datetime.day,
                    datetime.hour, datetime.minute, datetime.second,
                    smoothed_ax, smoothed_ay, smoothed_az,
                    peak_accel, peak_brake, peak_left, peak_right);
            fflush(logFile);
        }
    }

    if (logFile) fclose(logFile);
}
