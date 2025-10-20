#include <stdio.h>
#include <math.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "TCA9554PWR.h"
#include "PCF85063.h"
#include "QMI8658.h"
#include "ST7701S.h"
#include "CST820.h"
#include "SD_SPI.h"
#include "LVGL_Driver.h"
#include "LVGL_Example.h"
#include "Speedo_demo.h"
#include "Wireless.h"

float ax = 0, ay = 0, az = 0;
FILE *logFile = NULL;

void getAccelerometerData() {
    QMI8658_Read_Accel(&ax, &ay, &az); // update global accel values
}

void app_main(void)
{   
    // ---------- Initialize hardware ----------
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

    // ---------- Initialize LVGL UI ----------void Build_UI(void) {
    ui_init();      // loads SquareLine layout
    make_dial();    // call your existing LVGL drawing code
}
    // ---------- Open log file on SD ----------
    logFile = fopen("/sdcard/gforce_log.csv", "w");
    if (logFile) {
        fprintf(logFile, "Timestamp,Ax,Ay,Az\n");
        fflush(logFile);
    } else {
        printf("⚠️ SD log file could not be opened.\n");
    }

    // ---------- Main Loop ----------
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz update rate

        lv_timer_handler();
        PCF85063_Read_Time(&datetime);
        getAccelerometerData();
        update_dial_marker(ax, ay);

        // ---------- Log to SD card ----------
        if (logFile) {
            // Format timestamp as HH:MM:SS
            fprintf(logFile, "%02d:%02d:%02d,%.3f,%.3f,%.3f\n",
                    datetime.hour, datetime.minute, datetime.second,
                    ax, ay, az);
            fflush(logFile); // ensure data is written to SD
        }
    }

    // ---------- Close file (normally never reached) ----------
    if (logFile) fclose(logFile);
}
