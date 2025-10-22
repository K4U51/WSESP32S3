#include "ui_hook.h"
#include "ui.h"        // SquareLine generated objects
#include "GForceUI.h"  // for ui_dot, ui_label_*, ui_gauge_*

void hook_gforce_ui(void) {
    // ---------- Assign gauges ----------
    ui_gauge_accel  = gauge_accel;  // Replace with actual object name from SquareLine
    ui_gauge_brake  = gauge_brake;
    ui_gauge_left   = gauge_left;
    ui_gauge_right  = gauge_right;

    // ---------- Assign labels ----------
    ui_label_accel       = label_accel;
    ui_label_brake       = label_brake;
    ui_label_left        = label_left;
    ui_label_right       = label_right;
    ui_label_peak_accel  = label_peak_accel;
    ui_label_peak_brake  = label_peak_brake;
    ui_label_peak_left   = label_peak_left;
    ui_label_peak_right  = label_peak_right;

    // ---------- Assign moving dot ----------
    ui_dot = dot_obj; // replace with actual LVGL object name for the dot
}
