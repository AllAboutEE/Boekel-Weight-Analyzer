#include <LiquidCrystal.h>
#include "DFR_Key.h"
#include "screen_draw.h"
#include "screen_actions.h"
#include <stdbool.h>

unsigned long timeStartCalibration = 0;
unsigned long timeCalibrationEnd = 0;

float * factor;

float factorOne = 1.000f;
float factorTwo = 1.000f;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
DFR_Key keypad;

int localKey = NO_KEY;
                 
void setup() 
{ 
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.clear();
  keypad.setRate(10);
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
        // float reading = getReading();

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

        drawEnterFactorOneScreen();
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
              setScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_PROMPT_SCREEN);
            }
            else if(checkScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_TWO_SCREEN))
            {
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
