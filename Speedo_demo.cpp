#include "Speedo_demo.h"   // Keep for LVGL context + fonts
#include <stdio.h>
#include <math.h>
#include "fonts/montserrat_bold_28.h"

// ---------- Global LVGL Objects ----------
lv_obj_t *scr;           // Main screen
lv_obj_t *dial_circle;   // Circular frame for reference
lv_obj_t *marker_obj;    // Moving G-force marker (dot)
lv_obj_t *label_g;       // Numeric G display

// ---------- Styles ----------
static lv_style_t style_label;
static lv_style_t style_marker;
static lv_style_t style_circle;

// ---------- Colors ----------
lv_color_t color_black = LV_COLOR_MAKE(0, 0, 0);
lv_color_t color_red   = LV_COLOR_MAKE(255, 60, 60);
lv_color_t color_white = LV_COLOR_MAKE(255, 255, 255);
lv_color_t color_amber = LV_COLOR_MAKE(250, 140, 0);

// ---------- Dial Parameters ----------
const int dial_size = 400;
const int dial_center_x = 240;
const int dial_center_y = 240;
const float dial_radius = 140.0f;

// ---------- Initialize Styles ----------
void make_styles(void) {
    // Label (G-force value)
    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label, &montserrat_bold_28);
    lv_style_set_text_color(&style_label, color_amber);

    // Marker (moving dot)
    lv_style_init(&style_marker);
    lv_style_set_bg_color(&style_marker, color_red);
    lv_style_set_bg_opa(&style_marker, LV_OPA_COVER);
    lv_style_set_radius(&style_marker, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_marker, 0);

    // Circle outline
    lv_style_init(&style_circle);
    lv_style_set_border_color(&style_circle, color_white);
    lv_style_set_border_width(&style_circle, 3);
    lv_style_set_radius(&style_circle, LV_RADIUS_CIRCLE);
    lv_style_set_bg_opa(&style_circle, LV_OPA_TRANSP);
}

// ---------- Create G-Force Dial ----------
void make_dial(void) {
    // Background circle (dial reference)
    dial_circle = lv_obj_create(scr);
    lv_obj_set_size(dial_circle, dial_size, dial_size);
    lv_obj_center(dial_circle);
    lv_obj_add_style(dial_circle, &style_circle, 0);
    lv_obj_clear_flag(dial_circle, LV_OBJ_FLAG_SCROLLABLE);

    // G-force numeric label
    label_g = lv_label_create(scr);
    lv_label_set_text(label_g, "0.00 G");
    lv_obj_add_style(label_g, &style_label, 0);
    lv_obj_align(label_g, LV_ALIGN_CENTER, 0, 160);

    // Moving marker
    marker_obj = lv_obj_create(scr);
    lv_obj_set_size(marker_obj, 22, 22);
    lv_obj_add_style(marker_obj, &style_marker, 0);
    lv_obj_set_pos(marker_obj, dial_center_x, dial_center_y);
}

// ---------- Update Marker Based on G-Force ----------
void update_gforce_marker(float ax, float ay) {
    // Clamp input range (±1.5g)
    if (ax > 1.5f) ax = 1.5f;
    if (ax < -1.5f) ax = -1.5f;
    if (ay > 1.5f) ay = 1.5f;
    if (ay < -1.5f) ay = -1.5f;

    // Map ±1.5g to dial radius
    float scale = dial_radius / 1.5f;
    int x = dial_center_x + (int)(ax * scale);
    int y = dial_center_y - (int)(ay * scale);

    // Move marker on display
    lv_obj_set_pos(marker_obj, x, y);

    // Update G label
    float g_total = sqrtf(ax * ax + ay * ay);
    static char g_text[16];
    snprintf(g_text, sizeof(g_text), "%.2f G", g_total);
    lv_label_set_text(label_g, g_text);
}

// ---------- Build Entire UI ----------
void Build_UI(void) {
    scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, color_black, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    make_styles();
    make_dial();
}
