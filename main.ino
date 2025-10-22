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

FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from PCF85063 RTC

#define UPDATE_RATE_MS 50

// ---------- Simple smoothing ----------
#define SMOOTH_FACTOR 0.2
float smoothed_ax = 0;
float smoothed_ay = 0;
float smoothed_az = 0;

// ---------- Map function ----------
int mapFloatToGauge(float value, float min_val, float max_val, int gauge_min, int gauge_max) {
    if (value < min_val) value = min_val;
    if (value > max_val) value = max_val;
    return (int)((value - min_val) * (gauge_max - gauge_min) / (max_val - min_val) + gauge_min);
}

// ---------- Helper: generate unique filename ----------
static void generate_log_filename(char *filename, int max_len) {
    PCF85063_Read_Time(&datetime); // read RTC first
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
    printf("✅ UI loaded.\n");

    if (ui_dot) lv_obj_set_pos(ui_dot, 240, 240);

    // ---------- Generate log filename ----------
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

        getAccelerometerData();    // Read accelerometer

        // ---------- Apply simple smoothing ----------
        smoothed_ax = smoothed_ax * (1.0 - SMOOTH_FACTOR) + ax * SMOOTH_FACTOR;
        smoothed_ay = smoothed_ay * (1.0 - SMOOTH_FACTOR) + ay * SMOOTH_FACTOR;
        smoothed_az = smoothed_az * (1.0 - SMOOTH_FACTOR) + az * SMOOTH_FACTOR;

        // ---------- Map values to gauge range ----------
        int gauge_ax = mapFloatToGauge(smoothed_ax, -2.5f, 2.5f, 0, 100);  // Adjust min/max to match your SquareLine gauge
        int gauge_ay = mapFloatToGauge(smoothed_ay, -2.5f, 2.5f, 0, 100);
        int gauge_az = mapFloatToGauge(smoothed_az, -2.5f, 2.5f, 0, 100);

        // ---------- Update UI ----------
        update_gforce_ui(smoothed_ax, smoothed_ay, smoothed_az);
        
        // Example: if you have a specific gauge object from SquareLine:
        if (ui_gauge) lv_gauge_set_value(ui_gauge, 0, gauge_ax); // primary gauge axis

        // ---------- Log to SD ----------
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
