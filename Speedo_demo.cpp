#include "Speedo_demo.h"
#include <stdio.h>
#include <math.h>
#include "fonts/montserrat_bold_28.h"

// ---------- Global LVGL Objects ----------
lv_obj_t *scr;          // Main screen
lv_obj_t *dial_arc;     // Outer dial ring
lv_obj_t *marker_obj;   // Moving G-force marker (dot)
lv_obj_t *label_g;      // Numeric G display

// ---------- Styles ----------
static lv_style_t style_unit_text;
static lv_style_t style_marker;

// ---------- Colors ----------
lv_color_t palette_amber = LV_COLOR_MAKE(250, 140, 0);
lv_color_t palette_black = LV_COLOR_MAKE(0, 0, 0);
lv_color_t palette_red   = LV_COLOR_MAKE(255, 0, 0);
lv_color_t palette_white = LV_COLOR_MAKE(255, 255, 255);

// ---------- Dial Parameters ----------
const int dial_size = 400;
const int dial_arc_width = 8;
const int dial_center_x = 240;
const int dial_center_y = 240;
const float dial_radius = 140.0;

// ---------- Initialize Styles ----------
void make_styles(void) {
    lv_style_init(&style_unit_text);
    lv_style_set_text_font(&style_unit_text, &montserrat_bold_28);
    lv_style_set_text_color(&style_unit_text, palette_amber);

    lv_style_init(&style_marker);
    lv_style_set_bg_color(&style_marker, palette_red);
    lv_style_set_bg_opa(&style_marker, LV_OPA_COVER);
    lv_style_set_radius(&style_marker, LV_RADIUS_CIRCLE);
}

// ---------- Create Dial ----------
void make_dial(void) {
    // Create dial arc (circle)
    dial_arc = lv_arc_create(scr);
    lv_obj_set_size(dial_arc, dial_size, dial_size);
    lv_obj_center(dial_arc);
    lv_arc_set_bg_angles(dial_arc, 0, 360);
    lv_obj_set_style_arc_color(dial_arc, palette_white, LV_PART_MAIN);
    lv_obj_set_style_arc_width(dial_arc, dial_arc_width, LV_PART_MAIN);
    lv_obj_remove_style(dial_arc, NULL, LV_PART_KNOB);
    lv_obj_remove_style(dial_arc, NULL, LV_PART_INDICATOR);

    // Create G-force label
    label_g = lv_label_create(scr);
    lv_label_set_text(label_g, "0.00 G");
    lv_obj_add_style(label_g, &style_unit_text, 0);
    lv_obj_align(label_g, LV_ALIGN_CENTER, 0, 150);

    // Create moving marker
    marker_obj = lv_obj_create(scr);
    lv_obj_set_size(marker_obj, 20, 20);
    lv_obj_add_style(marker_obj, &style_marker, 0);
    lv_obj_set_style_border_width(marker_obj, 0, 0);
    lv_obj_set_style_bg_opa(marker_obj, LV_OPA_COVER, 0);
    lv_obj_set_pos(marker_obj, dial_center_x, dial_center_y);
}

// ---------- Update Marker Based on G-Force ----------
void update_dial_marker(float ax, float ay) {
    // Clamp values for visualization
    if (ax > 1.5) ax = 1.5;
    if (ax < -1.5) ax = -1.5;
    if (ay > 1.5) ay = 1.5;
    if (ay < -1.5) ay = -1.5;

    // Map ±1.5g to dial radius
    float scale = dial_radius / 1.5;
    int x = dial_center_x + (int)(ax * scale);
    int y = dial_center_y - (int)(ay * scale);

    // Move marker on screen
    lv_obj_set_pos(marker_obj, x, y);

    // Calculate magnitude (total G)
    float g_total = sqrtf(ax * ax + ay * ay);
    static char g_text[16];
    snprintf(g_text, sizeof(g_text), "%.2f G", g_total);
    lv_label_set_text(label_g, g_text);
}

// ---------- Build Entire UI ----------
void Build_UI(void) {
    scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, palette_black, 0);
    make_styles();
    make_dial();
}
