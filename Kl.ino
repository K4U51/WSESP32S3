#include <stdio.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "CST820.h"
#include "SD_SPI.h"
#include "LVGL_Driver.h"
#include "Wireless.h"
#include "Speedo_demo.h" // <- your LVGL UI from SquareLine

// ---------- Global Variables ----------
float ax = 0, ay = 0, az = 0;
FILE *logFile = NULL;
extern RTC_DateTypeDef datetime;  // from PCF85063 RTC

// ---------- Accelerometer Reading ----------
void getAccelerometerData() {
    QMI8658_Read_Accel(&ax, &ay, &az);
}

// ---------- LVGL Setup ----------
void Build_UI(void) {
    ui_init();        // Loads SquareLine project layout
    make_dial();      // Draws custom dial graphics
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

        // Update LVGL gauge or dial position
        update_dial_marker(ax, ay);

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
