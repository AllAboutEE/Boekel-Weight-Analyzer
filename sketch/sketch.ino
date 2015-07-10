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
const int EEPROM_ADDRESS_KVALUE = EEPROM_ADDRESS_CALIBRATION_TWO_VOLTAGE + sizeof(float);
const int EEPROM_ADDRESS_BVALUE = EEPROM_ADDRESS_KVALUE + sizeof(float);

float factorOne = 1.000f;
float factorTwo = 1.000f;
float calibrationOneVoltage = 0.000f;
float calibrationTwoVoltage = 0.000f;
float kvalue = 0.000f;
float bvalue = 0.000f;

float rawToVoltage(float raw);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
DFR_Key keypad;
HX711  scale(A2,A1);

int localKey = NO_KEY;

// variables used to calculate current reading (cr)
float crVoltageSum[100] = {0};
unsigned long crAverageCount = 0;
unsigned long crTimeStart = 0;

// variables used to get calibration voltages
float calibrationVoltageSum[100] = {0};
unsigned long calibrationAverageCount = 0;
                 
void setup() 
{ 
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.clear();
  keypad.setRate(10);

  scale.set_scale();
  scale.tare();
  
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
       crTimeStart = millis();
       crAverageCount = 0;
       updateCRMainScreen("--.--");
       delay(1000); // "debounce" button for when user is coming from another screen via a button press.

        EEPROM.get(EEPROM_ADDRESS_FACTOR_ONE,factorOne);
        EEPROM.get(EEPROM_ADDRESS_FACTOR_TWO,factorTwo); 
        
        EEPROM.get(EEPROM_ADDRESS_CALIBRATION_ONE_VOLTAGE,calibrationOneVoltage);
        EEPROM.get(EEPROM_ADDRESS_CALIBRATION_TWO_VOLTAGE,calibrationTwoVoltage); 
        
        EEPROM.get(EEPROM_ADDRESS_KVALUE,kvalue);
        EEPROM.get(EEPROM_ADDRESS_BVALUE,bvalue);

        
        Serial.print("Factor One: ");  
        Serial.println(factorOne,3);
        
        Serial.print("Factor Two: ");
        Serial.println(factorTwo,3);
        
        Serial.print("Calibration One Voltage (mV): ");
        Serial.println(calibrationOneVoltage,8);
        
        Serial.print("Calibration Two Voltage (mV): ");
        Serial.println(calibrationTwoVoltage,8);
        
        Serial.print("K Value: ");
        Serial.println(kvalue,4);
        
        Serial.print("B Value: ");
        Serial.println(bvalue,4);

        Serial.println();
   }
   else
   {
        // User is in the main screen. The CR value is been updated.
        if( ((crTimeStart + crAverageCount*1000)<=millis()) && crAverageCount<=100) // a second has passed
        {
          crVoltageSum[crAverageCount] = rawToVoltage(scale.read());
          crAverageCount++;
        }
        
        
        // TODO obtain reading
        if(crAverageCount==100)
        {
          crTimeStart = millis();
          crAverageCount = 0;

          float cW = currentWeight(kvalue,getAverage(crVoltageSum,100),bvalue);
          float cR = currentReading(factorTwo,cW);
          updateCRMainScreen(cR); // update current reading on screen.
          Serial.print("Current Weight: ");
          Serial.println(cW,4);
          Serial.print("Current Reading: ");
          Serial.println(cR,4);
          Serial.println();
        } 
        
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
      calibrationAverageCount = 0;
    }
    else
    {
      // calibration starts
      while(millis() < timeCalibrationEnd) 
      {       
        // update time on screen
        unsigned long timeRemainingMs = timeCalibrationEnd - millis();
        unsigned long timeRemainingMinutes = timeRemainingMs / 1000UL / 60UL;
        unsigned long timeRemainingSeconds = (timeRemainingMs/1000UL) - (timeRemainingMinutes * 60UL);
        
        char timeRemainingString[5] = {'\0'};

        // User is in the main screen. The CR value is been updated.

        if(timeRemainingMs <= (105*1000)) // read the last 100 readings, compensate with 5 for delays
        {
          if( ((timeStartCalibration + calibrationAverageCount*600)<=millis()) && calibrationAverageCount<=100) // a second has passed
          {
            calibrationVoltageSum[calibrationAverageCount] = rawToVoltage(scale.read());
            calibrationAverageCount++;
          }
        }
        
        sprintf(timeRemainingString, "%lu:%.2lu", timeRemainingMinutes,timeRemainingSeconds);
        updateFirstCalibrationRemaningTime(timeRemainingString);
      }
      // calibration ends
      
      clearScreenActionFlags();
      if(checkScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_IN_PROGRESS_SCREEN))
      {
        calibrationOneVoltage = getAverage(calibrationVoltageSum,100);
        EEPROM.put(EEPROM_ADDRESS_CALIBRATION_ONE_VOLTAGE,calibrationOneVoltage);
        setScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_ONE_SCREEN);
        calibrationAverageCount = 0;
      }
      else if(checkScreenDrawFlag(FLAG_DRAW_2ND_CALIBRATION_IN_PROGRESS_SCREEN))
      {
        calibrationTwoVoltage = getAverage(calibrationVoltageSum,100);
        EEPROM.put(EEPROM_ADDRESS_CALIBRATION_TWO_VOLTAGE,calibrationTwoVoltage);
        setScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_TWO_SCREEN);
        calibrationAverageCount = 0;
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
            factorOne = 1.00f;
            
            factor = &factorOne;
        }
        else if(checkScreenDrawFlag(FLAG_DRAW_PROMPT_FOR_FACTOR_TWO_SCREEN))
        {
            factorTwo = 1.00f;
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
              EEPROM.put(EEPROM_ADDRESS_FACTOR_TWO,factorTwo);

              EEPROM.get(EEPROM_ADDRESS_FACTOR_ONE,factorOne);
              EEPROM.get(EEPROM_ADDRESS_FACTOR_TWO,factorTwo); 
              
              EEPROM.get(EEPROM_ADDRESS_CALIBRATION_ONE_VOLTAGE,calibrationOneVoltage);
              EEPROM.get(EEPROM_ADDRESS_CALIBRATION_TWO_VOLTAGE,calibrationTwoVoltage); 

              kvalue = (factorTwo - factorOne) / ( (1.00f/calibrationTwoVoltage) - (1.00f/calibrationOneVoltage));
              EEPROM.put(EEPROM_ADDRESS_KVALUE, kvalue);
              
              bvalue = factorOne - (kvalue/calibrationOneVoltage);
              EEPROM.put(EEPROM_ADDRESS_BVALUE, bvalue);
              
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

float currentWeight(float Kvalue, float VoltageAverage, float Bvalue)
{
  return ((Kvalue)/(VoltageAverage))+Bvalue;
}

float currentReading(float factorTwo, float currentWeight)
{
  return ((factorTwo - currentWeight)/8.0f)*1000.0f;
}

float getAverage(float * values, int length)
{
  float sum = 0;
  
  for(int i = 0; i<length; i++)
  {
    sum += *(values+i);
  }
  
  return sum/(float)length;
}


