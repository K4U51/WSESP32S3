#pragma once
#include "lvgl.h"

// ---------- Accelerometer / G-Force data ----------
extern float ax, ay, az;
extern float smoothed_ax, smoothed_ay;
extern float peak_accel, peak_brake, peak_left, peak_right;

// ---------- LVGL Objects (initialized to nullptr) ----------
extern lv_obj_t *ui_dot;
extern lv_obj_t *ui_label_left;
extern lv_obj_t *ui_label_right;
extern lv_obj_t *ui_label_accel;
extern lv_obj_t *ui_label_brake;
extern lv_obj_t *ui_label_peak_accel;
extern lv_obj_t *ui_label_peak_brake;
extern lv_obj_t *ui_label_peak_left;
extern lv_obj_t *ui_label_peak_right;

extern lv_obj_t *ui_gauge_accel;
extern lv_obj_t *ui_gauge_brake;
extern lv_obj_t *ui_gauge_left;
extern lv_obj_t *ui_gauge_right;

// ---------- Functions ----------
void getAccelerometerData(void);
void update_gforce_ui(float ax_val, float ay_val, float az_val);
