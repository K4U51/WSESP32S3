#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "CST820.h"

#include "Wireless.h"
#include "Gyro_QMI8658.h"
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include "LVGL_Driver.h"
#include "BAT_Driver.h"

#include "ui.h"   // ← SquareLine Studio generated header

// ---------- Global Variables ----------
float ax = 0, ay = 0, az = 0;
float smoothed_ax = 0, smoothed_ay = 0;
FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from PCF85063 RTC

// ---------- LVGL Object References ----------
extern lv_obj_t *ui_dot;
extern lv_obj_t *ui_dial_image;
extern lv_obj_t *ui_label_left;
extern lv_obj_t *ui_label_right;
extern lv_obj_t *ui_label_accel;
extern lv_obj_t *ui_label_brake;

// ---------- Dial Settings ----------
#define DIAL_CENTER_X 240
#define DIAL_CENTER_Y 240
#define DIAL_SCALE    90.0f
#define UPDATE_RATE_MS 50

// ---------- Lerp Factor (0–1: smaller = smoother) ----------
#define LERP_FACTOR 0.15f

// ---------- Helper: Linear Interpolation ----------
static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// ---------- Read Accelerometer Data ----------
void getAccelerometerData() {
    QMI8658_Read_Accel(&ax, &ay, &az);
}

// ---------- Update G-Force Visualization ----------
void update_gforce_ui(float ax, float ay, float az) {
    // Clamp ±1.5g
    if (ax > 1.5f) ax = 1.5f;
    if (ax < -1.5f) ax = -1.5f;
    if (ay > 1.5f) ay = 1.5f;
    if (ay < -1.5f) ay = -1.5f;

    // Apply smoothing
    smoothed_ax = lerp(smoothed_ax, ax, LERP_FACTOR);
    smoothed_ay = lerp(smoothed_ay, ay, LERP_FACTOR);

    // Convert G-force → pixel displacement
    int16_t dot_x = DIAL_CENTER_X + (int16_t)(smoothed_ax * DIAL_SCALE);
    int16_t dot_y = DIAL_CENTER_Y - (int16_t)(smoothed_ay * DIAL_SCALE);

    // Move dot
    if (ui_dot)
        lv_obj_set_pos(ui_dot, dot_x, dot_y);

    // Update labels for live values
    char buf[16];

    if (ui_label_accel) {
        sprintf(buf, "%.2f", smoothed_ay > 0 ? smoothed_ay : 0);
        lv_label_set_text(ui_label_accel, buf);
    }
    if (ui_label_brake) {
        sprintf(buf, "%.2f", smoothed_ay < 0 ? -smoothed_ay : 0);
        lv_label_set_text(ui_label_brake, buf);
    }
    if (ui_label_left) {
        sprintf(buf, "%.2f", smoothed_ax < 0 ? -smoothed_ax : 0);
        lv_label_set_text(ui_label_left, buf);
    }
    if (ui_label_right) {
        sprintf(buf, "%.2f", smoothed_ax > 0 ? smoothed_ax : 0);
        lv_label_set_text(ui_label_right, buf);
    }
}

// ---------- Main Application ----------
void app_main(void)
{
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

    // ---------- Initialize SquareLine UI ----------
    ui_init();
    printf("✅ UI loaded.\n");

    // Center dot initially
    if (ui_dot)
        lv_obj_set_pos(ui_dot, DIAL_CENTER_X, DIAL_CENTER_Y);

    // ---------- Open SD Log ----------
    logFile = fopen("/sdcard/gforce_log.csv", "w");
    if (logFile) {
        fprintf(logFile, "Timestamp,Ax,Ay,Az\n");
        fflush(logFile);
        printf("✅ Logging started: /sdcard/gforce_log.csv\n");
    } else {
        printf("⚠️ Could not open SD log file.\n");
    }

    // ---------- Main Loop ----------
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(UPDATE_RATE_MS));

        lv_timer_handler();
        PCF85063_Read_Time(&datetime);
        getAccelerometerData();

        // Update LVGL visualization
        update_gforce_ui(ax, ay, az);

        // Log to SD
        if (logFile) {
            fprintf(logFile, "%02d:%02d:%02d,%.3f,%.3f,%.3f\n",
                    datetime.hour, datetime.minute, datetime.second,
                    ax, ay, az);
            fflush(logFile);
        }
    }

    if (logFile)
        fclose(logFile);
}
