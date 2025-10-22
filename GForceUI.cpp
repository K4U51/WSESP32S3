#include "GForceUI.h"
#include "QMI8658.h"
#include <stdio.h>

// Accelerometer readings
float ax = 0, ay = 0, az = 0;
float smoothed_ax = 0, smoothed_ay = 0;

// Peak tracking
float peak_accel = 0, peak_brake = 0, peak_left = 0, peak_right = 0;

// LVGL object pointers (hooked from ui_hook)
lv_obj_t *ui_dot = NULL;
lv_obj_t *ui_label_left = NULL;
lv_obj_t *ui_label_right = NULL;
lv_obj_t *ui_label_accel = NULL;
lv_obj_t *ui_label_brake = NULL;
lv_obj_t *ui_label_peak_accel = NULL;
lv_obj_t *ui_label_peak_brake = NULL;
lv_obj_t *ui_label_peak_left = NULL;
lv_obj_t *ui_label_peak_right = NULL;
lv_obj_t *ui_gauge_accel = NULL;
lv_obj_t *ui_gauge_brake = NULL;
lv_obj_t *ui_gauge_left = NULL;
lv_obj_t *ui_gauge_right = NULL;

#define DIAL_CENTER_X 240
#define DIAL_CENTER_Y 240
#define DIAL_SCALE    90.0f
#define LERP_FACTOR   0.15f
#define G_MAX         2.5f

static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static int mapGToGauge(float g_val, int gauge_min = 0, int gauge_max = 100) {
    if (g_val < -G_MAX) g_val = -G_MAX;
    if (g_val >  G_MAX) g_val =  G_MAX;
    return (int)((g_val + G_MAX) * (gauge_max - gauge_min) / (2.0f * G_MAX) + gauge_min);
}

void getAccelerometerData(void) {
    QMI8658_Read_Accel(&ax, &ay, &az);
}

void update_gforce_ui(float ax_val, float ay_val, float az_val) {
    // Clamp
    if (ax_val > G_MAX) ax_val = G_MAX;
    if (ax_val < -G_MAX) ax_val = -G_MAX;
    if (ay_val > G_MAX) ay_val = G_MAX;
    if (ay_val < -G_MAX) ay_val = -G_MAX;

    // Smooth readings
    smoothed_ax = lerp(smoothed_ax, ax_val, LERP_FACTOR);
    smoothed_ay = lerp(smoothed_ay, ay_val, LERP_FACTOR);

    // Update dial dot
    int16_t dot_x = DIAL_CENTER_X + (int16_t)(smoothed_ax * DIAL_SCALE);
    int16_t dot_y = DIAL_CENTER_Y - (int16_t)(smoothed_ay * DIAL_SCALE);
    if (ui_dot) lv_obj_set_pos(ui_dot, dot_x, dot_y);

    char buf[16];

    // Live labels
    if (ui_label_accel) lv_label_set_text_fmt(ui_label_accel, "%.2f", smoothed_ay > 0 ? smoothed_ay : 0);
    if (ui_label_brake) lv_label_set_text_fmt(ui_label_brake, "%.2f", smoothed_ay < 0 ? -smoothed_ay : 0);
    if (ui_label_left) lv_label_set_text_fmt(ui_label_left, "%.2f", smoothed_ax < 0 ? -smoothed_ax : 0);
    if (ui_label_right) lv_label_set_text_fmt(ui_label_right, "%.2f", smoothed_ax > 0 ? smoothed_ax : 0);

    // Peak tracking
    if (smoothed_ay > peak_accel) peak_accel = smoothed_ay;
    if (smoothed_ay < -peak_brake) peak_brake = -smoothed_ay;
    if (smoothed_ax < -peak_left) peak_left = -smoothed_ax;
    if (smoothed_ax > peak_right) peak_right = smoothed_ax;

    // Peak labels
    if (ui_label_peak_accel) lv_label_set_text_fmt(ui_label_peak_accel, "%.2f", peak_accel);
    if (ui_label_peak_brake) lv_label_set_text_fmt(ui_label_peak_brake, "%.2f", peak_brake);
    if (ui_label_peak_left) lv_label_set_text_fmt(ui_label_peak_left, "%.2f", peak_left);
    if (ui_label_peak_right) lv_label_set_text_fmt(ui_label_peak_right, "%.2f", peak_right);

    // Update SquareLine gauges
    if (ui_gauge_accel) lv_gauge_set_value(ui_gauge_accel, 0, mapGToGauge(smoothed_ay));
    if (ui_gauge_brake) lv_gauge_set_value(ui_gauge_brake, 0, mapGToGauge(-smoothed_ay));
    if (ui_gauge_left) lv_gauge_set_value(ui_gauge_left, 0, mapGToGauge(-smoothed_ax));
    if (ui_gauge_right) lv_gauge_set_value(ui_gauge_right, 0, mapGToGauge(smoothed_ax));
}
