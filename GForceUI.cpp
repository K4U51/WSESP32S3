#include "GForceUI.h"
#include "ui_hook.h"
#include "ui.h"  // SquareLine generated objects
#include <stdio.h>
#include <math.h>

#define DIAL_CENTER_X 240
#define DIAL_CENTER_Y 240
#define DIAL_SCALE    90.0f
#define LERP_FACTOR   0.15f
#define G_MAX         2.5f  // Maximum G for mapping

// ---------- Linear interpolation ----------
static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Map ±G to gauge integer range
static int mapGToGauge(float g_val, int gauge_min, int gauge_max) {
    if (g_val < -G_MAX) g_val = -G_MAX;
    if (g_val >  G_MAX) g_val =  G_MAX;
    return (int)((g_val + G_MAX) * (gauge_max - gauge_min) / (2.0f * G_MAX) + gauge_min);
}

// ---------- Read accelerometer data ----------
void getAccelerometerData(void) {
    QMI8658_Read_Accel(&ax, &ay, &az);
}

// ---------- Update LVGL UI ----------
void update_gforce_ui(float ax_val, float ay_val, float az_val) {
    // Clamp to ±G_MAX
    if (ax_val > G_MAX) ax_val = G_MAX;
    if (ax_val < -G_MAX) ax_val = -G_MAX;
    if (ay_val > G_MAX) ay_val = G_MAX;
    if (ay_val < -G_MAX) ay_val = -G_MAX;

    // Smooth readings
    smoothed_ax = lerp(smoothed_ax, ax_val, LERP_FACTOR);
    smoothed_ay = lerp(smoothed_ay, ay_val, LERP_FACTOR);

    // ---------- Update moving dot ----------
    if (ui_dot) {
        int16_t dot_x = DIAL_CENTER_X + (int16_t)(smoothed_ax * DIAL_SCALE);
        int16_t dot_y = DIAL_CENTER_Y - (int16_t)(smoothed_ay * DIAL_SCALE);
        lv_obj_set_pos(ui_dot, dot_x, dot_y);
    }

    char buf[16];

    // ---------- Update live labels ----------
    if (ui_label_accel) sprintf(buf, "%.2f", smoothed_ay > 0 ? smoothed_ay : 0), lv_label_set_text(ui_label_accel, buf);
    if (ui_label_brake) sprintf(buf, "%.2f", smoothed_ay < 0 ? -smoothed_ay : 0), lv_label_set_text(ui_label_brake, buf);
    if (ui_label_left)  sprintf(buf, "%.2f", smoothed_ax < 0 ? -smoothed_ax : 0), lv_label_set_text(ui_label_left, buf);
    if (ui_label_right) sprintf(buf, "%.2f", smoothed_ax > 0 ? smoothed_ax : 0), lv_label_set_text(ui_label_right, buf);

    // ---------- Update peak values ----------
    if (smoothed_ay > peak_accel) peak_accel = smoothed_ay;
    if (smoothed_ay < -peak_brake) peak_brake = -smoothed_ay;
    if (smoothed_ax < -peak_left) peak_left = -smoothed_ax;
    if (smoothed_ax > peak_right) peak_right = smoothed_ax;

    if (ui_label_peak_accel) sprintf(buf, "%.2f", peak_accel), lv_label_set_text(ui_label_peak_accel, buf);
    if (ui_label_peak_brake) sprintf(buf, "%.2f", peak_brake), lv_label_set_text(ui_label_peak_brake, buf);
    if (ui_label_peak_left)  sprintf(buf, "%.2f", peak_left), lv_label_set_text(ui_label_peak_left, buf);
    if (ui_label_peak_right) sprintf(buf, "%.2f", peak_right), lv_label_set_text(ui_label_peak_right, buf);

    // ---------- Update SquareLine gauges ----------
    if (ui_gauge_accel) lv_gauge_set_value(ui_gauge_accel, 0, mapGToGauge(smoothed_ay, 0, 100));
    if (ui_gauge_brake) lv_gauge_set_value(ui_gauge_brake, 0, mapGToGauge(-smoothed_ay, 0, 100));
    if (ui_gauge_left)  lv_gauge_set_value(ui_gauge_left, 0, mapGToGauge(-smoothed_ax, 0, 100));
    if (ui_gauge_right) lv_gauge_set_value(ui_gauge_right, 0, mapGToGauge(smoothed_ax, 0, 100));
}
