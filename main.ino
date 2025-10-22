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
#include "ui_hook.h"      // Hook LVGL objects to GForceUI

FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from PCF85063 RTC

extern float peak_accel, peak_brake, peak_left, peak_right; // Peaks from GForceUI

#define UPDATE_RATE_MS 50

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
    hook_gforce_ui();   // Connect SquareLine LVGL objects to GForceUI
    if (ui_dot) lv_obj_set_pos(ui_dot, DIAL_CENTER_X, DIAL_CENTER_Y);
    printf("✅ UI loaded and hooks applied.\n");

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

        // ---------- Update UI ----------
        update_gforce_ui(ax, ay, az);  // GForceUI handles smoothing & gauge updates

        // ---------- Log to SD ----------
        if (logFile) {
            fprintf(logFile, "%04d-%02d-%02d %02d:%02d:%02d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                    datetime.year, datetime.month, datetime.day,
                    datetime.hour, datetime.minute, datetime.second,
                    ax, ay, az,
                    peak_accel, peak_brake, peak_left, peak_right);
            fflush(logFile);
        }
    }

    if (logFile) fclose(logFile);
}
