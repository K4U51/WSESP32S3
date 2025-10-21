#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "CST820.h"

#include "Speedo_demo.h"  // SquareLine UI
#include "Wireless.h"
#include "Gyro_QMI8658.h"
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include "LVGL_Driver.h"
#include "LVGL_Example.h"
#include "BAT_Driver.h"

// ---------- Global Variables ----------
float ax = 0, ay = 0, az = 0;
float smoothed_ax = 0, smoothed_ay = 0;
FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from PCF85063 RTC

// LVGL UI objects
static lv_obj_t *gforce_dot;
static lv_obj_t *gforce_screen;

// ---------- Accelerometer Reading ----------
void getAccelerometerData() {
    QMI8658_Read_Accel(&ax, &ay, &az);
}

// ---------- G-Force Dot Update ----------
void update_dot_position(float ax, float ay) {
    if (!gforce_dot) return;

    // Apply smoothing (simple low-pass filter)
    const float alpha = 0.2f;  // 0.0 = very smooth, 1.0 = no smoothing
    smoothed_ax = smoothed_ax * (1.0f - alpha) + ax * alpha;
    smoothed_ay = smoothed_ay * (1.0f - alpha) + ay * alpha;

    // Scale for display motion
    float scale = 40.0f;
    int x_offset = (int)(smoothed_ax * scale);
    int y_offset = (int)(-smoothed_ay * scale);  // invert Y to match display

    lv_obj_align(gforce_dot, LV_ALIGN_CENTER, x_offset, y_offset);
}

// ---------- LVGL Setup ----------
void Build_UI(void) {
    ui_init();        // Loads SquareLine layout
    make_dial();      // Custom dial graphics if any

    gforce_screen = lv_scr_act();

    // Create red dot
    gforce_dot = lv_obj_create(gforce_screen);
    lv_obj_set_size(gforce_dot, 10, 10);
    lv_obj_set_style_bg_color(gforce_dot, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_clear_flag(gforce_dot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(gforce_dot, LV_ALIGN_CENTER, 0, 0);
}

// ---------- Main Application ----------
void app_main(void)
{
    // ---------- Initialize Hardware ----------
    Wireless_Init();
    Flash_Searching();
    I2C_Init();
    PCF85063_Init();
    QMI8658_Init();
    EXIO_Init();
    LCD_Init();
    Touch_Init();
    SD_Init();
    LVGL_Init();

    // ---------- Initialize UI ----------
    Build_UI();

    // ---------- Open SD Log ----------
    logFile = fopen("/sdcard/gforce_log.csv", "w");
    if (logFile) {
        fprintf(logFile, "Timestamp,Ax,Ay,Az\n");
        fflush(logFile);
        printf("✅ Logging started: /sdcard/gforce_log.csv\n");
    } else {
        printf("⚠️ SD log file could not be opened.\n");
    }

    // ---------- Main Loop ----------
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz refresh

        lv_timer_handler();
        PCF85063_Read_Time(&datetime);
        getAccelerometerData();

        // Update LVGL dot position (smoothed)
        update_dot_position(ax, ay);

        // ---------- Log to SD ----------
        if (logFile) {
            fprintf(logFile, "%02d:%02d:%02d,%.3f,%.3f,%.3f\n",
                    datetime.hour, datetime.minute, datetime.second,
                    ax, ay, az);
            fflush(logFile);
        }
    }

    // ---------- Close file (never reached normally) ----------
    if (logFile) fclose(logFile);
}
