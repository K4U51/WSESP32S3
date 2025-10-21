#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "CST820.h"

#include "Speedo_demo.h"   // <- G-Force UI header (Build_UI + update_gforce_marker)

#include "Wireless.h"
#include "Gyro_QMI8658.h"
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include "LVGL_Driver.h"
#include "BAT_Driver.h"

// ---------- Global Variables ----------
float ax = 0, ay = 0, az = 0;
FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from PCF85063 RTC

// ---------- Read Accelerometer Data ----------
void getAccelerometerData() {
    QMI8658_Read_Accel(&ax, &ay, &az);
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
    Build_UI();  // from Speedo_demo.h (your G-Force dial)

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
        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz update

        lv_timer_handler();             // LVGL refresh
        PCF85063_Read_Time(&datetime);  // Update RTC
        getAccelerometerData();         // Read accelerometer

        // Update LVGL visualization
        update_gforce_marker(ax, ay);

        // ---------- Log to SD ----------
        if (logFile) {
            fprintf(logFile, "%02d:%02d:%02d,%.3f,%.3f,%.3f\n",
                    datetime.hour, datetime.minute, datetime.second,
                    ax, ay, az);
            fflush(logFile);
        }
    }

    // ---------- Cleanup (never normally reached) ----------
    if (logFile) fclose(logFile);
}
