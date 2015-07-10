#ifndef __SCREEN_DRAW_H__
#define __SCREEN_DRAW_H__

#include <Arduino.h>

class LiquidCrystal;
// screen draw flags
const uint8_t FLAG_DRAW_MAIN_SCREEN = 0x01;
const uint8_t FLAG_DRAW_BOOT_SCREEN = 0x02;
const uint8_t FLAG_DRAW_CONFIRM_CALIBRATION_SCREEN = 0x03;
const uint8_t FLAG_DRAW_1ST_CALIBRATION_PROMPT_SCREEN = 0x04;
const uint8_t FLAG_DRAW_1ST_CALIBRATION_IN_PROGRESS_SCREEN = 0x05;
const uint8_t FLAG_DRAW_PROMPT_FOR_FACTOR_ONE_SCREEN = 0x06;

const uint8_t FLAG_DRAW_2ND_CALIBRATION_PROMPT_SCREEN = 0x08;
const uint8_t FLAG_DRAW_2ND_CALIBRATION_IN_PROGRESS_SCREEN = 0x10;
const uint8_t FLAG_DRAW_PROMPT_FOR_FACTOR_TWO_SCREEN = 0x20;

bool checkScreenDrawFlag(const uint8_t FLAG);
uint8_t getScreenDrawFlags();
uint8_t setScreenDrawFlag(const uint8_t FLAG);
uint8_t clearScreenDrawFlag(const uint8_t FLAG);
void clearScreenDrawFlags();
void drawBootScreen();
void drawMainScreen();
void drawConfirmCalibrationScreen(uint8_t selection);
void drawCalibrationPromptScreen(const char * string);
void drawCalibrationInProgessScreen();
void drawEnterFactorOneScreen(char * factor);

#endif
