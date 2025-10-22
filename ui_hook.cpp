#include "ui_hook.h"
#include "ui.h"       // SquareLine generated UI
#include "GForceUI.h" // Your modular G-Force handling

void hook_gforce_ui(void) {
    // --------- Assign LVGL objects from SquareLine UI ---------
    ui_dot               = dot_obj;          // Replace with actual SquareLine object names
    ui_label_accel        = label_accel;
    ui_label_brake        = label_brake;
    ui_label_left         = label_left;
    ui_label_right        = label_right;
    ui_label_peak_accel   = label_peak_accel;
    ui_label_peak_brake   = label_peak_brake;
    ui_label_peak_left    = label_peak_left;
    ui_label_peak_right   = label_peak_right;

    ui_gauge_accel        = gauge_accel;
    ui_gauge_brake        = gauge_brake;
    ui_gauge_left         = gauge_left;
    ui_gauge_right        = gauge_right;
}
