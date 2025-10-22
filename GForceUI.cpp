#include "GForceUI.h"
#include "QMI8658.h"
#include <stdio.h>

float ax = 0, ay = 0, az = 0;
float smoothed_ax = 0, smoothed_ay = 0;
float peak_accel = 0, peak_brake = 0, peak_left = 0, peak_right = 0;

// LVGL object pointers should be initialized from ui_init() in main
lv_obj_t *ui_dot = NULL;
lv_obj_t *ui_label_left = NULL;
lv_obj_t *ui_label_right = NULL;
lv_obj_t *ui_label_accel = NULL;
lv_obj_t *ui_label_brake = NULL;
lv_obj_t *ui_label_peak_accel = NULL;
lv_obj_t *ui_label_peak_brake = NULL;
lv_obj_t *ui_label_peak_left = NULL;
lv_obj_t *ui_label_peak_right = NULL;

#define DIAL_CENTER_X 240
#define DIAL_CENTER_Y 240
#define DIAL_SCALE    90.0f
#define LERP_FACTOR 0.15f

static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void getAccelerometerData(void) {
    QMI8658_Read_Accel(&ax, &ay, &az);
}

void update_gforce_ui(float ax_val, float ay_val, float az_val) {
    // Clamp ±1.5g
    if (ax_val > 1.5f) ax_val = 1.5f;
    if (ax_val < -1.5f) ax_val = -1.5f;
    if (ay_val > 1.5f) ay_val = 1.5f;
    if (ay_val < -1.5f) ay_val = -1.5f;

    // Smooth readings
    smoothed_ax = lerp(smoothed_ax, ax_val, LERP_FACTOR);
    smoothed_ay = lerp(smoothed_ay, ay_val, LERP_FACTOR);

    // Update dial dot
    int16_t dot_x = DIAL_CENTER_X + (int16_t)(smoothed_ax * DIAL_SCALE);
    int16_t dot_y = DIAL_CENTER_Y - (int16_t)(smoothed_ay * DIAL_SCALE);
    if (ui_dot) lv_obj_set_pos(ui_dot, dot_x, dot_y);

    char buf[16];

    // Live labels
    if (ui_label_accel) { sprintf(buf, "%.2f", smoothed_ay > 0 ? smoothed_ay : 0); lv_label_set_text(ui_label_accel, buf); }
    if (ui_label_brake) { sprintf(buf, "%.2f", smoothed_ay < 0 ? -smoothed_ay : 0); lv_label_set_text(ui_label_brake, buf); }
    if (ui_label_left) { sprintf(buf, "%.2f", smoothed_ax < 0 ? -smoothed_ax : 0); lv_label_set_text(ui_label_left, buf); }
    if (ui_label_right) { sprintf(buf, "%.2f", smoothed_ax > 0 ? smoothed_ax : 0); lv_label_set_text(ui_label_right, buf); }

    // Peak tracking
    if (smoothed_ay > peak_accel) peak_accel = smoothed_ay;
    if (smoothed_ay < -peak_brake) peak_brake = -smoothed_ay;
    if (smoothed_ax < -peak_left) peak_left = -smoothed_ax;
    if (smoothed_ax > peak_right) peak_right = smoothed_ax;

    // Peak labels
    if (ui_label_peak_accel) { sprintf(buf, "%.2f", peak_accel); lv_label_set_text(ui_label_peak_accel, buf); }
    if (ui_label_peak_brake) { sprintf(buf, "%.2f", peak_brake); lv_label_set_text(ui_label_peak_brake, buf); }
    if (ui_label_peak_left) { sprintf(buf, "%.2f", peak_left); lv_label_set_text(ui_label_peak_left, buf); }
    if (ui_label_peak_right) { sprintf(buf, "%.2f", peak_right); lv_label_set_text(ui_label_peak_right, buf); }
}
