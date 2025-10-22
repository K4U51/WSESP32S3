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
#define TRAIL_POINTS  30

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

// ---------- Internal state ----------
static float smoothed_ax = 0.0f;
static float smoothed_ay = 0.0f;
static float peak_accel = 0.0f;
static float peak_brake = 0.0f;
static float peak_left = 0.0f;
static float peak_right = 0.0f;

static lv_point_t trail_points[TRAIL_POINTS];
static uint8_t trail_index = 0;

// ---------- Read accelerometer data ----------
void getAccelerometerData(void) {
    QMI8658_Read_Accel(&ax, &ay, &az);
}

// ---------- Draw fading G-trail ----------
static void draw_gforce_trail(lv_obj_t *canvas) {
    if (!canvas) return;
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

    for (int i = 0; i < TRAIL_POINTS; i++) {
        int idx = (trail_index + i) % TRAIL_POINTS;
        lv_point_t p = trail_points[idx];

        uint8_t fade = (uint8_t)(255 - (i * (255 / TRAIL_POINTS)));
        lv_canvas_set_px(canvas, p.x, p.y, lv_color_make(255, 255, fade));  // yellowish fade
    }
}

// ---------- Update LVGL UI ----------
void update_gforce_ui(float ax_val, float ay_val, float az_val) {
    // Clamp to ±G_MAX
    ax_val = fmaxf(fminf(ax_val, G_MAX), -G_MAX);
    ay_val = fmaxf(fminf(ay_val, G_MAX), -G_MAX);

    // Smooth readings
    smoothed_ax = lerp(smoothed_ax, ax_val, LERP_FACTOR);
    smoothed_ay = lerp(smoothed_ay, ay_val, LERP_FACTOR);

    // Compute position
    int16_t dot_x = DIAL_CENTER_X + (int16_t)(smoothed_ax * DIAL_SCALE);
    int16_t dot_y = DIAL_CENTER_Y - (int16_t)(smoothed_ay * DIAL_SCALE);

    // Save to trail
    trail_points[trail_index] = {dot_x, dot_y};
    trail_index = (trail_index + 1) % TRAIL_POINTS;

    // Draw trail
    draw_gforce_trail(ui_canvas_gtrace);

    // ---------- Update moving dot ----------
    if (ui_dot) {
        // Calculate G magnitude for color
        float g_magnitude = sqrtf(smoothed_ax * smoothed_ax + smoothed_ay * smoothed_ay);
        float intensity = fminf(g_magnitude / G_MAX, 1.0f);

        uint8_t r = (uint8_t)(intensity * 255);
        uint8_t g = (uint8_t)(255 - intensity * 128);  // fade from green→yellow→red
        uint8_t b = 0;

        lv_color_t color = lv_color_make(r, g, b);
        lv_obj_set_style_bg_color(ui_dot, color, 0);

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

    // ---------- Update SquareLine gauges (optional) ----------
    if (ui_gauge_accel) lv_gauge_set_value(ui_gauge_accel, 0, mapGToGauge(smoothed_ay, 0, 100));
    if (ui_gauge_brake) lv_gauge_set_value(ui_gauge_brake, 0, mapGToGauge(-smoothed_ay, 0, 100));
    if (ui_gauge_left)  lv_gauge_set_value(ui_gauge_left, 0, mapGToGauge(-smoothed_ax, 0, 100));
    if (ui_gauge_right) lv_gauge_set_value(ui_gauge_right, 0, mapGToGauge(smoothed_ax, 0, 100));
}
