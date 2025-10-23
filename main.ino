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
#define G_MAX 2.5f    // Max G for mapping dial
#define DIAL_CENTER_X 240
#define DIAL_CENTER_Y 240
#define DIAL_SCALE    90.0f
#define TRAIL_LENGTH 10

float smoothed_ax = 0;
float smoothed_ay = 0;
float smoothed_az = 0;

// Peak values
float peak_accel = 0;
float peak_brake = 0;
float peak_left  = 0;
float peak_right = 0;

// Linear interpolation
static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// SD log filename generator
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

// ---------- Trail system ----------
typedef struct { int16_t x, y; } DotPos;
static DotPos trail[TRAIL_LENGTH];
static int trail_index = 0;
lv_obj_t* ui_dot_trail[TRAIL_LENGTH];  // Trail LVGL objects

// Initialize trail objects
void initGForceTrail(lv_obj_t* parent) {
    for (int i = 0; i < TRAIL_LENGTH; i++) {
        ui_dot_trail[i] = lv_obj_create(parent);
        lv_obj_set_size(ui_dot_trail[i], 6, 6);
        lv_obj_set_style_radius(ui_dot_trail[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(ui_dot_trail[i], lv_color_hex(0x00FF00), 0);
        lv_obj_set_style_bg_opa(ui_dot_trail[i], 0, 0); // invisible initially
        lv_obj_clear_flag(ui_dot_trail[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(ui_dot_trail[i], LV_ALIGN_TOP_LEFT, DIAL_CENTER_X, DIAL_CENTER_Y);
    }
}

// Update function for dot + trail + labels
void updateGForceUI(float ax, float ay, float az) {
    // Circular mapping
    float mag = sqrtf(ax*ax + ay*ay);
    if (mag > G_MAX) {
        ax = (ax / mag) * G_MAX;
        ay = (ay / mag) * G_MAX;
    }

    static int16_t last_x = DIAL_CENTER_X;
    static int16_t last_y = DIAL_CENTER_Y;

    int16_t target_x = DIAL_CENTER_X + (int16_t)((ax / G_MAX) * DIAL_SCALE);
    int16_t target_y = DIAL_CENTER_Y - (int16_t)((ay / G_MAX) * DIAL_SCALE);

    int16_t dot_x = (int16_t)lerp(last_x, target_x, SMOOTH_FACTOR);
    int16_t dot_y = (int16_t)lerp(last_y, target_y, SMOOTH_FACTOR);

    last_x = dot_x;
    last_y = dot_y;

    // Move main dot
    if (ui_dot) lv_obj_set_pos(ui_dot, dot_x, dot_y);

    // Shift trail positions
    trail[trail_index].x = dot_x;
    trail[trail_index].y = dot_y;
    trail_index = (trail_index + 1) % TRAIL_LENGTH;

    // Update trail objects
    for (int i = 0; i < TRAIL_LENGTH; i++) {
        int idx = (trail_index + i) % TRAIL_LENGTH;
        int alpha = (i + 1) * (255 / TRAIL_LENGTH);
        if (ui_dot_trail[i]) {
            lv_obj_set_pos(ui_dot_trail[i], trail[idx].x, trail[idx].y);
            lv_obj_set_style_opa(ui_dot_trail[i], alpha, 0);
        }
    }

    // Update labels
    if (ui_label_gx) lv_label_set_text_fmt(ui_label_gx, "X: %.2f g", ax);
    if (ui_label_gy) lv_label_set_text_fmt(ui_label_gy, "Y: %.2f g", ay);
    if (ui_label_gz) lv_label_set_text_fmt(ui_label_gz, "Z: %.2f g", az);
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
    hook_gforce_ui();
    initGForceTrail(lv_scr_act()); // Create trail objects
    printf("✅ UI loaded and hooks applied.\n");

    // Reset peak values
    peak_accel = peak_brake = peak_left = peak_right = 0;

    // Initial dot position
    if (ui_dot) lv_obj_set_pos(ui_dot, DIAL_CENTER_X, DIAL_CENTER_Y);

    // SD log filename
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

        lv_timer_handler(); // Refresh LVGL

        // Read accelerometer
        getAccelerometerData();

        // Smooth accelerometer
        smoothed_ax = smoothed_ax * (1.0 - SMOOTH_FACTOR) + ax * SMOOTH_FACTOR;
        smoothed_ay = smoothed_ay * (1.0 - SMOOTH_FACTOR) + ay * SMOOTH_FACTOR;
        smoothed_az = smoothed_az * (1.0 - SMOOTH_FACTOR) + az * SMOOTH_FACTOR;

        // Update peaks
        if (smoothed_ax > peak_accel) peak_accel = smoothed_ax;
        if (smoothed_ax < -peak_brake) peak_brake = -smoothed_ax;
        if (smoothed_ay < -peak_left)  peak_left  = -smoothed_ay;
        if (smoothed_ay > peak_right)  peak_right = smoothed_ay;

        // Update dot, trail, and labels
        updateGForceUI(smoothed_ax, smoothed_ay, smoothed_az);

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
