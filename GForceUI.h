#pragma once
#include "lvgl.h"
#include "RTC_PCF85063.h"

// ---------- Live G-Force readings ----------
extern float ax;
extern float ay;
extern float az;

// ---------- Smoothed values ----------
extern float smoothed_ax;
extern float smoothed_ay;

// ---------- Peak values ----------
extern float peak_accel;
extern float peak_brake;
extern float peak_left;
extern float peak_right;

// ---------- LVGL Object References ----------
extern lv_obj_t *ui_dot;
extern lv_obj_t *ui_label_left;
extern lv_obj_t *ui_label_right;
extern lv_obj_t *ui_label_accel;
extern lv_obj_t *ui_label_brake;
extern lv_obj_t *ui_label_peak_accel;
extern lv_obj_t *ui_label_peak_brake;
extern lv_obj_t *ui_label_peak_left;
extern lv_obj_t *ui_label_peak_right;

// ---------- Functions ----------
void getAccelerometerData(void);
void update_gforce_ui(float ax, float ay, float az);
