#pragma once

#include "lvgl.h"
#include <stdio.h>
#include <math.h>

// ---------- LVGL Tick Configuration ----------
#define EXAMPLE1_LVGL_TICK_PERIOD_MS 1000

// ---------- UI Function Prototypes ----------

// Initializes styles and builds all LVGL objects for the G-force screen
void Build_UI(void);

// Updates the moving G-force marker and numeric label based on accelerometer data
void update_gforce_marker(float ax, float ay);
