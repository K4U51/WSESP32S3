#include "ui.h"
#include "GForceUI.h"
#include "lvgl.h"

// Create LVGL objects (placeholders)
lv_obj_t *gauge_accel = nullptr;
lv_obj_t *gauge_brake = nullptr;
lv_obj_t *gauge_left  = nullptr;
lv_obj_t *gauge_right = nullptr;

lv_obj_t *label_accel = nullptr;
lv_obj_t *label_brake = nullptr;
lv_obj_t *label_left  = nullptr;
lv_obj_t *label_right = nullptr;

lv_obj_t *label_peak_accel = nullptr;
lv_obj_t *label_peak_brake = nullptr;
lv_obj_t *label_peak_left  = nullptr;
lv_obj_t *label_peak_right = nullptr;

lv_obj_t *dot_obj = nullptr;

void ui_main_screen_init(void) {
    // ---------- Create demo screen ----------
    lv_obj_t *scr = lv_scr_act();

    // Gauges
    gauge_accel = lv_gauge_create(scr);
    gauge_brake = lv_gauge_create(scr);
    gauge_left  = lv_gauge_create(scr);
    gauge_right = lv_gauge_create(scr);

    // Live labels
    label_accel = lv_label_create(scr);
    label_brake = lv_label_create(scr);
    label_left  = lv_label_create(scr);
    label_right = lv_label_create(scr);

    // Peak labels
    label_peak_accel = lv_label_create(scr);
    label_peak_brake = lv_label_create(scr);
    label_peak_left  = lv_label_create(scr);
    label_peak_right = lv_label_create(scr);

    // Dot
    dot_obj = lv_obj_create(scr);
    lv_obj_set_size(dot_obj, 10, 10);
    lv_obj_set_style_bg_color(dot_obj, lv_color_make(255,0,0), 0);

    // ---------- Hook objects to GForceUI ----------
    ui_gauge_accel = gauge_accel;
    ui_gauge_brake = gauge_brake;
    ui_gauge_left  = gauge_left;
    ui_gauge_right = gauge_right;

    ui_label_accel = label_accel;
    ui_label_brake = label_brake;
    ui_label_left  = label_left;
    ui_label_right = label_right;

    ui_label_peak_accel = label_peak_accel;
    ui_label_peak_brake = label_peak_brake;
    ui_label_peak_left  = label_peak_left;
    ui_label_peak_right = label_peak_right;

    ui_dot = dot_obj;

    // Center dot initially
    lv_obj_set_pos(dot_obj, 240, 240);

    printf("✅ Demo UI initialized with GForce hooks.\n");
}
