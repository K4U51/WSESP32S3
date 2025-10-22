#include "Speedo_demo.h"
#include "fonts/montserrat_bold_28.h"
#include <math.h>

// ---------- Global LVGL Objects ----------
lv_obj_t *scr;           // Main screen
lv_obj_t *marker_obj;    // Moving G-force marker (dot)

// ---------- Axis labels ----------
static lv_obj_t *label_left;
static lv_obj_t *label_right;
static lv_obj_t *label_accel;
static lv_obj_t *label_brake;

// ---------- Styles ----------
static lv_style_t style_label;
static lv_style_t style_marker;

// ---------- Colors ----------
lv_color_t color_black = LV_COLOR_MAKE(0, 0, 0);
lv_color_t color_red   = LV_COLOR_MAKE(255, 60, 60);
lv_color_t color_amber = LV_COLOR_MAKE(250, 140, 0);

// ---------- Dial Parameters ----------
const int dial_center_x = 240;
const int dial_center_y = 240;
const float dial_radius = 140.0f;

// ---------- Image Background ----------
LV_IMG_DECLARE(dial_bg_img); // Convert your dial image to LVGL array

// ---------- Initialize Styles ----------
void make_styles(void) {
    // Label style
    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label, &montserrat_bold_28);
    lv_style_set_text_color(&style_label, color_amber);

    // Marker style
    lv_style_init(&style_marker);
    lv_style_set_bg_color(&style_marker, color_red);
    lv_style_set_bg_opa(&style_marker, LV_OPA_COVER);
    lv_style_set_radius(&style_marker, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_marker, 0);
}

// ---------- Create G-Force Dial ----------
void make_dial(void) {
    // Background image
    lv_obj_t *dial_img = lv_img_create(scr);
    lv_img_set_src(dial_img, &dial_bg_img);
    lv_obj_center(dial_img);
    lv_obj_clear_flag(dial_img, LV_OBJ_FLAG_SCROLLABLE);

    // Axis labels
    label_left = lv_label_create(scr);
    lv_label_set_text(label_left, "0.00");
    lv_obj_add_style(label_left, &style_label, 0);
    lv_obj_align(label_left, LV_ALIGN_CENTER, -dial_radius, 0);

    label_right = lv_label_create(scr);
    lv_label_set_text(label_right, "0.00");
    lv_obj_add_style(label_right, &style_label, 0);
    lv_obj_align(label_right, LV_ALIGN_CENTER, dial_radius, 0);

    label_accel = lv_label_create(scr);
    lv_label_set_text(label_accel, "0.00");
    lv_obj_add_style(label_accel, &style_label, 0);
    lv_obj_align(label_accel, LV_ALIGN_CENTER, 0, -dial_radius);

    label_brake = lv_label_create(scr);
    lv_label_set_text(label_brake, "0.00");
    lv_obj_add_style(label_brake, &style_label, 0);
    lv_obj_align(label_brake, LV_ALIGN_CENTER, 0, dial_radius);

    // Moving marker
    marker_obj = lv_obj_create(scr);
    lv_obj_set_size(marker_obj, 22, 22);
    lv_obj_add_style(marker_obj, &style_marker, 0);
    lv_obj_set_pos(marker_obj, dial_center_x, dial_center_y);
}

// ---------- Update Marker and Labels ----------
void update_gforce_marker(float ax, float ay) {
    // Clamp ±1.5G for display
    if (ax > 1.5f) ax = 1.5f;
    if (ax < -1.5f) ax = -1.5f;
    if (ay > 1.5f) ay = 1.5f;
    if (ay < -1.5f) ay = -1.5f;

    // Map ±1.5G to dial radius
    float scale = dial_radius / 1.5f;
    int x = dial_center_x + (int)(ax * scale);
    int y = dial_center_y - (int)(ay * scale);

    // Move marker
    lv_obj_set_pos(marker_obj, x, y);

    // Update axis labels
    char text[16];
    snprintf(text, sizeof(text), "%.2f", (ax < 0) ? -ax : 0.0f);
    lv_label_set_text(label_left, text);

    snprintf(text, sizeof(text), "%.2f", (ax > 0) ? ax : 0.0f);
    lv_label_set_text(label_right, text);

    snprintf(text, sizeof(text), "%.2f", (ay > 0) ? ay : 0.0f);
    lv_label_set_text(label_accel, text);

    snprintf(text, sizeof(text), "%.2f", (ay < 0) ? -ay : 0.0f);
    lv_label_set_text(label_brake, text);
}

// ---------- Build Entire UI ----------
void Build_UI(void) {
    scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, color_black, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    make_styles();
    make_dial();
}
