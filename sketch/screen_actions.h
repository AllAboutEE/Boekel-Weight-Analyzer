#ifndef __SCREEN_ACTIONS_H__
#define __SCREEN_ACTIONS_H__

#include <Arduino.h>

class LiquidCrystal;

// screen action flags
const uint8_t FLAG_NO_SCREEN_ACTION = 0X00;
const uint8_t FLAG_SCREEN_ACTION_UPDATE_CR_MAIN_SCREEN = 0x01;
const uint8_t FLAG_SCREEN_ACTION_CONFIRM_CALIBRATION_SELECTION = 0x02;
const uint8_t FLAG_SCREEN_ACTION_CONFIRM_CALIBRATION = 0x04;
const uint8_t FLAG_SCREEN_ACTION_START_CALIBRATION = 0x08;
const uint8_t FLAG_SCREEN_ACTION_ENTER_FACTOR = 0x10;

// used for factor one and two assignment
const uint8_t PLACE_ONES = 0;
const uint8_t PLACE_TENTHS = 1;
const uint8_t PLACE_HUNDREDTHS = 2;
const uint8_t PLACE_THOUSANDTHS = 3;

bool checkScreenActionFlag(const uint8_t FLAG);
uint8_t getScreenActionFlags();
void setScreenActionFlag(const uint8_t FLAG);
void clearScreenActionFlag(const uint8_t FLAG);
void clearScreenActionFlags();
void setFactor(float value, float *factor);
void incrementFactor(float * factor);
void decrementFactor(float * factor);
void setFactorCursor(uint8_t place);
void movePlaceRight();
void movePlaceLeft();
void setPlace(const uint8_t p);
uint8_t getPlace();
void resetPlace();
void updateCRMainScreen(float reading);
void updateCRMainScreen(const char * s);
void updateFirstCalibrationRemaningTime(char * timeRemaining);

#endif

