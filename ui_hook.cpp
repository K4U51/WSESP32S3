#include "ui_hook.h"
#include <stdio.h>

void hook_gforce_ui(void) {
    // Hook SquareLine objects to GForceUI pointers
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

    // Optional: initialize positions/values
    if (ui_dot) lv_obj_set_pos(ui_dot, 240, 240);
    if (ui_gauge_accel) lv_gauge_set_value(ui_gauge_accel, 0, 0);
    if (ui_gauge_brake) lv_gauge_set_value(ui_gauge_brake, 0, 0);
    if (ui_gauge_left)  lv_gauge_set_value(ui_gauge_left, 0, 0);
    if (ui_gauge_right) lv_gauge_set_value(ui_gauge_right, 0, 0);

    printf("✅ GForceUI hooks assigned.\n");
}
