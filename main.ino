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

float peak_accel = 0;
float peak_brake = 0;
float peak_left = 0;
float peak_right = 0;

FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from PCF85063 RTC

// ---------- LVGL Object References ----------
extern lv_obj_t *ui_dot;
extern lv_obj_t *ui_dial_image;
extern lv_obj_t *ui_label_left;
extern lv_obj_t *ui_label_right;
extern lv_obj_t *ui_label_accel;
extern lv_obj_t *ui_label_brake;

// Peak labels (add these in SquareLine and export)
extern lv_obj_t *ui_label_peak_accel;
extern lv_obj_t *ui_label_peak_brake;
extern lv_obj_t *ui_label_peak_left;
extern lv_obj_t *ui_label_peak_right;

// ---------- Dial Settings ----------
#define DIAL_CENTER_X 240
#define DIAL_CENTER_Y 240
#define DIAL_SCALE    90.0f
#define UPDATE_RATE_MS 50
#define LERP_FACTOR 0.15f

// ---------- Helper: Linear Interpolation ----------
static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// ---------- Read Accelerometer Data ----------
void getAccelerometerData() {
    QMI8658_Read_Accel(&ax, &ay, &az);
}

// ---------- Update G-Force UI and Peaks ----------
void update_gforce_ui(float ax, float ay, float az) {
    // Clamp ±1.5g
    if (ax > 1.5f) ax = 1.5f;
    if (ax < -1.5f) ax = -1.5f;
    if (ay > 1.5f) ay = 1.5f;
    if (ay < -1.5f) ay = -1.5f;

    // Smooth readings
    smoothed_ax = lerp(smoothed_ax, ax, LERP_FACTOR);
    smoothed_ay = lerp(smoothed_ay, ay, LERP_FACTOR);

    // Update dial dot
    int16_t dot_x = DIAL_CENTER_X + (int16_t)(smoothed_ax * DIAL_SCALE);
    int16_t dot_y = DIAL_CENTER_Y - (int16_t)(smoothed_ay * DIAL_SCALE);
    if (ui_dot) lv_obj_set_pos(ui_dot, dot_x, dot_y);

    // Update live labels
    char buf[16];
    if (ui_label_accel) { sprintf(buf, "%.2f", smoothed_ay > 0 ? smoothed_ay : 0); lv_label_set_text(ui_label_accel, buf); }
    if (ui_label_brake) { sprintf(buf, "%.2f", smoothed_ay < 0 ? -smoothed_ay : 0); lv_label_set_text(ui_label_brake, buf); }
    if (ui_label_left) { sprintf(buf, "%.2f", smoothed_ax < 0 ? -smoothed_ax : 0); lv_label_set_text(ui_label_left, buf); }
    if (ui_label_right) { sprintf(buf, "%.2f", smoothed_ax > 0 ? smoothed_ax : 0); lv_label_set_text(ui_label_right, buf); }

    // Update peaks
    if (smoothed_ay > peak_accel) peak_accel = smoothed_ay;
    if (smoothed_ay < -peak_brake) peak_brake = -smoothed_ay;
    if (smoothed_ax < -peak_left) peak_left = -smoothed_ax;
    if (smoothed_ax > peak_right) peak_right = smoothed_ax;

    // Update peak labels
    if (ui_label_peak_accel) { sprintf(buf, "%.2f", peak_accel); lv_label_set_text(ui_label_peak_accel, buf); }
    if (ui_label_peak_brake) { sprintf(buf, "%.2f", peak_brake); lv_label_set_text(ui_label_peak_brake, buf); }
    if (ui_label_peak_left) { sprintf(buf, "%.2f", peak_left); lv_label_set_text(ui_label_peak_left, buf); }
    if (ui_label_peak_right) { sprintf(buf, "%.2f", peak_right); lv_label_set_text(ui_label_peak_right, buf); }
}

// ---------- Main App ----------
void app_main(void)
{
    printf("🚀 Starting G-Force UI with SquareLine...\n");

    // Hardware init
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

    // UI init
    ui_init();
    printf("✅ UI loaded.\n");
    if (ui_dot) lv_obj_set_pos(ui_dot, DIAL_CENTER_X, DIAL_CENTER_Y);

    // Read RTC for filename
    PCF85063_Read_Time(&datetime);

    // Generate unique filename with session counter
    char filename[128];
    int session = 1;
    do {
        sprintf(filename, "/sdcard/gforce_%04d%02d%02d_%02d%02d%02d_%d.csv",
                datetime.year, datetime.month, datetime.day,
                datetime.hour, datetime.minute, datetime.second,
                session);
        FILE *file = fopen(filename, "r");
        if (file) { fclose(file); session++; if(session > 999) break; } else { break; }
    } while(1);

    // Open log file
    logFile = fopen(filename, "w");
    if (logFile) {
        fprintf(logFile, "Timestamp,Ax,Ay,Az,PeakAccel,PeakBrake,PeakLeft,PeakRight\n");
        fflush(logFile);
        printf("✅ Logging started: %s\n", filename);
    } else {
        printf("⚠️ Could not open SD log file: %s\n", filename);
    }

    // Main loop
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(UPDATE_RATE_MS));

        lv_timer_handler();             // Refresh LVGL
        PCF85063_Read_Time(&datetime);  // Update RTC
        getAccelerometerData();         // Read accelerometer

        // Update UI and peaks
        update_gforce_ui(ax, ay, az);

        // Log to SD including peaks
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
