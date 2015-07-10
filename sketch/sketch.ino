#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "DFR_Key.h"
#include "screen_draw.h"
#include "screen_actions.h"
#include "HX711.h"
#include <stdbool.h>

unsigned long timeStartCalibration = 0;
unsigned long timeCalibrationEnd = 0;

float * factor;

const int EEPROM_ADDRESS_FACTOR_ONE = 0;
const int EEPROM_ADDRESS_FACTOR_TWO = EEPROM_ADDRESS_FACTOR_ONE + sizeof(float);
const int EEPROM_ADDRESS_CALIBRATION_ONE_VOLTAGE = EEPROM_ADDRESS_FACTOR_TWO + sizeof(float);
const int EEPROM_ADDRESS_CALIBRATION_TWO_VOLTAGE = EEPROM_ADDRESS_CALIBRATION_ONE_VOLTAGE + sizeof(float);

float factorOne = 1.000f;
float factorTwo = 1.000f;

float rawToVoltage(float raw);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
DFR_Key keypad;
HX711  scale(A2,A1);

int localKey = NO_KEY;
                 
void setup() 
{ 
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.clear();
  keypad.setRate(10);

  scale.set_scale();
  scale.tare();

  EEPROM.get(EEPROM_ADDRESS_FACTOR_ONE, factorOne);
  EEPROM.get(EEPROM_ADDRESS_FACTOR_TWO,factorTwo);

  
}

void loop()
{  
  if(checkScreenDrawFlag(FLAG_DRAW_BOOT_SCREEN))
  {
    // This is the boot screen where the software version number is displayed.
        
    drawBootScreen();
    clearScreenDrawFlags();
    setScreenDrawFlag(FLAG_DRAW_MAIN_SCREEN);
  }
  else if(checkScreenDrawFlag(FLAG_DRAW_MAIN_SCREEN))
  {
   // The user is in the main screen.
   
   if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_UPDATE_CR_MAIN_SCREEN))
   {
       drawMainScreen();
       setScreenActionFlag(FLAG_SCREEN_ACTION_UPDATE_CR_MAIN_SCREEN);
       delay(1000); // "debounce" button for when user is coming from another screen via a button press.
   }
   else
   {
        // User is in the main screen. The CR value is been updated.

        // TODO obtain reading
        Serial.println("Voltage: ");
        Serial.println(rawToVoltage(scale.read()));
        delay(1000);
        Serial.println("RAW: ");
        Serial.println(scale.read());
        delay(1000);
        
        updateCRMainScreen(20.50); // TODO pass reading instead of literal

        localKey = keypad.getKey();
        if(localKey!=SAMPLE_WAIT)
        {
            // The user wants to calibrate the unit. We need to verify the decision by entering the confirm calibration screen.
            if(localKey == SELECT_KEY)
            {
                clearScreenActionFlags();
                clearScreenDrawFlags();
                setScreenDrawFlag(FLAG_DRAW_CONFIRM_CALIBRATION_SCREEN);
            }
        }
   }
  }
  else if(checkScreenDrawFlag(FLAG_DRAW_CONFIRM_CALIBRATION_SCREEN))
  {
    // This is the screen where the user is given the option of procedding with the calibration or exiting/canceling.
    
    static uint8_t selection = 0;
    
    if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_CONFIRM_CALIBRATION_SELECTION))
    {
      // draw the screen text
      selection = 0;
      drawConfirmCalibrationScreen(selection);
      setScreenActionFlag(FLAG_SCREEN_ACTION_CONFIRM_CALIBRATION_SELECTION);
      delay(1000); // "debounce" button for when user is coming from another screen via a button press.
    }
    else
    {
      localKey = keypad.getKey();
      if(localKey!=SAMPLE_WAIT)
      {
        if(localKey == UP_KEY)
        {
          // Set selection to first row
          selection = 0;
          drawConfirmCalibrationScreen(selection);
        }
        else if(localKey == DOWN_KEY)
        {
          // Set selection to second row
          selection = 1;
          drawConfirmCalibrationScreen(selection);
        }
        else if(localKey == SELECT_KEY)
        {
         // choose selection
         
         clearScreenActionFlags();
         
         if(selection == 0)
         {
           // user wants to proceed with calibration of weight analyzer
           setScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_PROMPT_SCREEN);
         }
         else if(selection == 1)
         {
           // user wants to cancel calibration (Exit) i.e. return to main screen
           setScreenDrawFlag(FLAG_DRAW_MAIN_SCREEN);
         }
        }
        else
        {
          // Some other key was pressed. Not necessary to do anything.
        }
      }
    }
  }
  else if(checkScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_PROMPT_SCREEN) || checkScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_PROMPT_SCREEN))
  {
    // This is the screen where the user is told that the calibration will begin when the "Select" button is pressed.
    
    if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_CONFIRM_CALIBRATION))
    {
      if(checkScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_PROMPT_SCREEN))
      { 
          drawCalibrationPromptScreen("1st Calibration");
      }
      else if(checkScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_PROMPT_SCREEN))
      {
          drawCalibrationPromptScreen("2nd Calibration");
      }

      setScreenActionFlag(FLAG_SCREEN_ACTION_CONFIRM_CALIBRATION);
      delay(500); // "debounce" button for when user is coming from another screen via a button press.
    }
    else
    {
      localKey = keypad.getKey();
      if(localKey!=SAMPLE_WAIT)
      {
        if(localKey == SELECT_KEY)
        {
          // The user has confirmed he wants to enter/start calibration.

          clearScreenActionFlags();

          // If the user is in the 1st/2nd calibraiton prompt go to the 1st/2nd calibration in progress screen
          if(checkScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_PROMPT_SCREEN))
          { 
              setScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_IN_PROGRESS_SCREEN);
          }
          else if(checkScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_PROMPT_SCREEN))
          {
              setScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_IN_PROGRESS_SCREEN);
          }
        }
      }
    }
  }
  else if(checkScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_IN_PROGRESS_SCREEN) || checkScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_IN_PROGRESS_SCREEN))
  {
    // Screen that tells the user the calibration is in progress.
    
    if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_START_CALIBRATION))
    {
      drawCalibrationInProgessScreen(); 
      setScreenActionFlag(FLAG_SCREEN_ACTION_START_CALIBRATION);
      delay(500); // "debounce" button for when user is coming from another screen via a button press.
      timeStartCalibration = millis();
      timeCalibrationEnd = timeStartCalibration + 2UL*60UL*1000UL; // calibration takes 2 minutes
    }
    else
    {
      // calibration starts
      while(millis() < timeCalibrationEnd) 
      {
        // TODO read value from sensor and add to average array every 600ms
        
        // update time on screen
        unsigned long timeRemainingMs = timeCalibrationEnd - millis();
        unsigned long timeRemainingMinutes = timeRemainingMs / 1000UL / 60UL;
        unsigned long timeRemainingSeconds = (timeRemainingMs/1000UL) - (timeRemainingMinutes * 60UL);
        
        char timeRemainingString[5] = {'\0'};
        
        sprintf(timeRemainingString, "%lu:%.2lu", timeRemainingMinutes,timeRemainingSeconds);
        updateFirstCalibrationRemaningTime(timeRemainingString);
      }
      // calibration ends
      
      clearScreenActionFlags();
      if(checkScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_IN_PROGRESS_SCREEN))
      {
        setScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_ONE_SCREEN);
      }
      else if(checkScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_IN_PROGRESS_SCREEN))
      {
        setScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_TWO_SCREEN);
      }
    }
  }
  else if(checkScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_ONE_SCREEN) || checkScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_TWO_SCREEN))
  {
    // Screen where the user enters the Factor (both One and Two)
    
    
    if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_ENTER_FACTOR))
    {
        if(checkScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_ONE_SCREEN))
        {
            factor = &factorOne;
        }
        else if(checkScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_TWO_SCREEN))
        {
            factor = &factorTwo;
        }

        char factorString[6] = {'\0'};
  
        // Can't sprintf a float so let's seperate it
        int d1 = *factor; // get the number to the left of the decimal
        int d2 = trunc(((*factor)-d1)*1000); // get the number to the right of the decimal
        sprintf(factorString,"%d.%.3d",d1,d2);
        
        drawEnterFactorOneScreen(factorString);
        setScreenActionFlag(FLAG_SCREEN_ACTION_ENTER_FACTOR);
    }
    else
    {
      localKey = keypad.getKey();
            
      if(localKey!=SAMPLE_WAIT)
      {
        switch(localKey)
        {
        case UP_KEY:
           // increment the number
           
           incrementFactor(factor);
           delay(500);
          break;
        case DOWN_KEY:
           // decrement the number

           decrementFactor(factor);
           delay(500);
          break;
        case LEFT_KEY:
            // Select the figure to the left
            movePlaceLeft();
            setFactorCursor(getPlace());
            delay(500);
          break;
        case RIGHT_KEY:
            // Select the figure to the right
            movePlaceRight();
            setFactorCursor(getPlace());
            delay(500);
          break;
        case SELECT_KEY:
          // The user has confirmed the factor value

            clearScreenActionFlags();

            if(checkScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_ONE_SCREEN))
            {
              EEPROM.put(EEPROM_ADDRESS_FACTOR_ONE,factorOne);
              setScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_PROMPT_SCREEN);
            }
            else if(checkScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_TWO_SCREEN))
            {
              EEPROM.put(EEPROM_ADDRESS_FACTOR_ONE,factorTwo);
              setScreenDrawFlag(FLAG_DRAW_MAIN_SCREEN);            
            }
          break;
        default:
            // nothing to be done
          break;
        }        
      }
    }
  }
}

/**
 * @description Used to convert a 24-bit 2's compliment number (800000h (MIN), 7FFFFFh (MAX) to a voltage (mV) value , where 0x800000 == -20mV and 0x7FFFFF = 20mV
 * +/- 20mV is used because the default gain of the HX711 is 128 which as per the datasheet is used for +/- 20mV.
 * @ param raw The raw 2's compliment value
 */
float rawToVoltage(float raw)
{
 
  float m = 0; // slope (m). We will use the equation y - y1 = m(x - x1) to obtain the voltage value.
  
  if(raw <= 0x7fffff) // [0 to 20mV]
  {
    // The two points (x1, y1), (x2, y2) for 0mV to 20mV are (0x00000, 0mV), and (0x7fffff, 20mV)
    
    m = (0 - 20.0f) / (0 - (float)0x7fffff); 
   
    return m * (raw - 0) + 0; // y = m (x - x1) + y1
  }
  else // (0 to -20mv]
  {    
    // The two points (x1, y1), (x2, y2) for 0mV to -20mV are (0x800000, -20mV), and (0xffffff, 0mV) 
    
    m = (-20.0 - 0) / ((float)0x800000 - (float)0xffffff); // slope (m)
    
    return m * (raw - (float)0x800000) - 20.0f; // y = m (x - x1) + y1
  }
  
}


