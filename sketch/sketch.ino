#include <LiquidCrystal.h>
#include <DFR_Key.h>
#include <stdbool.h>

// screen draw flags
const uint8_t FLAG_DRAW_MAIN_SCREEN = 0x01;
const uint8_t FLAG_DRAW_BOOT_SCREEN = 0x02;
const uint8_t FLAG_DRAW_CONFIRM_CALIBRATION_SCREEN = 0x04;
const uint8_t FLAG_DRAW_1ST_CALIBRATION_PROMPT_SCREEN = 0x08;
const uint8_t FLAG_DRAW_1ST_CALIBRATION_IN_PROGRESS_SCREEN = 0x10;
const uint8_t FLAG_DRAW_FACTOR_ONE_SCREEN = 0x20;

uint8_t flagsScreenDraw = FLAG_DRAW_FACTOR_ONE_SCREEN;

// screen action flags
const uint8_t FLAG_NO_SCREEN_ACTION = 0X00;
const uint8_t FLAG_SCREEN_ACTION_UPDATE_CR_MAIN_SCREEN = 0x01;
const uint8_t FLAG_SCREEN_ACTION_CONFIRM_CALIBRATION_SELECTION = 0x02;
const uint8_t FLAG_SCREEN_ACTION_CONFIRM_FIRST_CALIBRATION = 0x04;
const uint8_t FLAG_SCREEN_ACTION_START_FIRST_CALIBRATION = 0x08;
const uint8_t FLAG_SCREEN_ACTION_ENTER_FACTOR_ONE = 0x10;

uint8_t flagsScreenAction = FLAG_NO_SCREEN_ACTION;


unsigned long timeStartCalibration = 0;
unsigned long timeCalibrationEnd = 0;

// used for factor one and two assignment
const uint8_t PLACE_ONES = 0;
const uint8_t PLACE_TENTHS = 1;
const uint8_t PLACE_HUNDREDTHS = 2;
const uint8_t PLACE_THOUSANDTHS = 3;

uint8_t place = PLACE_ONES;

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
        
    drawBootScreen(lcd);
    clearScreenDrawFlags();
    setScreenDrawFlag(FLAG_DRAW_MAIN_SCREEN);
  }
  else if(checkScreenDrawFlag(FLAG_DRAW_MAIN_SCREEN))
  {
   // The user is in the main screen.
   
   if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_UPDATE_CR_MAIN_SCREEN))
   {
       drawMainScreen(lcd);
       setScreenActionFlag(FLAG_SCREEN_ACTION_UPDATE_CR_MAIN_SCREEN);
       delay(1000); // "debounce" button for when user is coming from another screen via a button press.
   }
   else
   {
        // User is in the main screen. The CR value is been updated.

        // TODO obtain reading
        // float reading = getReading();

        updateCRMainScreen(lcd,20.50); // TODO pass reading instead of literal

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
      drawConfirmCalibrationScreen(lcd,selection);
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
          drawConfirmCalibrationScreen(lcd,selection);
        }
        else if(localKey == DOWN_KEY)
        {
          // Set selection to second row
          selection = 1;
          drawConfirmCalibrationScreen(lcd,selection);
        }
        else if(localKey == SELECT_KEY)
        {
         // choose selection
         clearScreenDrawFlags();
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
  else if(checkScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_PROMPT_SCREEN))
  {
    // This is the screen where the user is told that the 1st calibration will begin when the "Select" button is pressed.
    
    if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_CONFIRM_FIRST_CALIBRATION))
    {
      draw1stCalibrationPromptScreen();
      setScreenActionFlag(FLAG_SCREEN_ACTION_CONFIRM_FIRST_CALIBRATION);
      delay(1000); // "debounce" button for when user is coming from another screen via a button press.
    }
    else
    {
      localKey = keypad.getKey();
      if(localKey!=SAMPLE_WAIT)
      {
        if(localKey == SELECT_KEY)
        {
          clearScreenDrawFlags();
          clearScreenActionFlags();
          setScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_IN_PROGRESS_SCREEN);
        }
      }
    }
  }
  else if(checkScreenDrawFlag(FLAG_DRAW_1ST_CALIBRATION_IN_PROGRESS_SCREEN))
  {
        
    if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_START_FIRST_CALIBRATION))
    {
      draw1stCalibrationInProgessScreen(); 
      setScreenActionFlag(FLAG_SCREEN_ACTION_START_FIRST_CALIBRATION);
      delay(1000); // "debounce" button for when user is coming from another screen via a button press.
      timeStartCalibration = millis();
      timeCalibrationEnd = timeStartCalibration + 2UL*60UL*1000UL; // calibration takes 2 minutes
      
    }
    else
    {
      // 1st calibration starts
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
      // 1st calibration ends
      clearScreenDrawFlags();
      clearScreenActionFlags();
      setScreenDrawFlag(FLAG_DRAW_FACTOR_ONE_SCREEN);
    }
  }
  else if(checkScreenDrawFlag(FLAG_DRAW_FACTOR_ONE_SCREEN))
  {
    // The screen where the user enters Factor One. This screen appears after the 1st calibration screen.
    if(!checkScreenActionFlag(FLAG_SCREEN_ACTION_ENTER_FACTOR_ONE))
    {
      drawEnterFactorOneScreen();
      setScreenActionFlag(FLAG_SCREEN_ACTION_ENTER_FACTOR_ONE);
    }
    else
    {
      localKey = keypad.getKey();
            
      if(localKey!=SAMPLE_WAIT)
      {
        switch(localKey)
        {
          case UP_KEY:
           // increment figure
           incrementFactor(&factorOne);
           delay(500);
          break;
          case DOWN_KEY:
           // decrement figure
           decrementFactor(&factorOne);
           delay(500);
          break;
          case LEFT_KEY:
            movePlaceLeft();
            setFactorCursor(getPlace());
            delay(500);
          break;
          case RIGHT_KEY:
            movePlaceRight();
            setFactorCursor(getPlace());
            delay(500);
          break;
          case SELECT_KEY:
            // save entered factor
          break;
          default:
          break;
        }        
      }
    }
  }
}

// screen draw functions

bool checkScreenDrawFlag(const uint8_t FLAG)
{
  if( (flagsScreenDraw & FLAG) == FLAG )
  {
   return true; 
  }
  return false;
}

uint8_t getScreenDrawFlags()
{
  return flagsScreenDraw;
}

uint8_t setScreenDrawFlag(const uint8_t FLAG)
{
  flagsScreenDraw = FLAG; // only one screen can be set at a time.
}

uint8_t clearScreenDrawFlag(const uint8_t FLAG)
{
  flagsScreenDraw &= ~ FLAG ;
}

void clearScreenDrawFlags()
{
    flagsScreenDraw = 0x00;
}

void drawBootScreen(LiquidCrystal lcd)
{
  // Dispaly Boot Screen for 5 seconds
  lcd.setCursor(0,0);
  lcd.print("Weight Analyzer");
  lcd.setCursor(0,1);
  lcd.print("Version 1.0"); // TODO define software version at top of file and pass to this line.
  delay(5000); 
}

void drawMainScreen(LiquidCrystal lcd)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Weight Analyzer");
  lcd.setCursor(0,1);
  lcd.print("CR: "); // Current Reading
  
}

void drawConfirmCalibrationScreen(LiquidCrystal lcd,uint8_t selection)
{
  lcd.clear();
  lcd.setCursor(0,0);
  
  if(selection == 0)
  {
    lcd.print("Calibrate WA *");
    lcd.setCursor(0,1);
    lcd.print("Exit");
  }
  else if(selection == 1)
  {
    lcd.print("Calibrate WA");
    lcd.setCursor(0,1);
    lcd.print("Exit *");  
  }
}

void draw1stCalibrationPromptScreen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1st Calibration");
  lcd.setCursor(0,1);
  lcd.print("Press \"Select\"");
  
}

void draw1stCalibrationInProgessScreen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrating!");
  lcd.setCursor(0,1);
  lcd.print("Don't touch");
}

void drawEnterFactorOneScreen()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Use U/D/L/R");
  lcd.setCursor(0,1);
  lcd.print("Factor: 1.000");
  lcd.setCursor(8,1);
  lcd.cursor();
  lcd.blink();
}

void updateFirstCalibrationRemaningTime(char * timeRemaining)
{
  lcd.setCursor(12,1);
  lcd.print(timeRemaining);
}

// screen action functions

bool checkScreenActionFlag(const uint8_t FLAG)
{
  if( (flagsScreenAction & FLAG) == FLAG )
  {
   return true; 
  }
  return false;
}

uint8_t getScreenActionFlags()
{
  return flagsScreenAction;
}

uint8_t setScreenActionFlag(const uint8_t FLAG)
{
  flagsScreenAction |= FLAG;
}

uint8_t clearScreenActionFlag(const uint8_t FLAG)
{
  flagsScreenAction &= ~ FLAG ;
}

void clearScreenActionFlags()
{
    flagsScreenAction = 0x00;
}

void updateCRMainScreen(LiquidCrystal lcd, float reading)
{
 lcd.setCursor(4,1);
 lcd.print(reading);
}

// enter factor functions

void setFactor(float value, float *factor)
{
  Serial.println(value);
  if( (value>=0.980) && (value<=1.500f) )
  {
    *factor = value;
    
  }
  else if(value<0.980)
  {
    *factor = 0.980f;
  }
  else if(value>1.500f)
  {
    *factor = 1.500f;
  }
}

void incrementFactor(float * factor)
{
  float change = 0.000;
  uint8_t place = getPlace();
  switch(place)
  {
    case PLACE_ONES:
      change = 1.000f;
    break;
    case PLACE_TENTHS:
      change = 0.100f;
    break;
    case PLACE_HUNDREDTHS:
      change = 0.010f;
    break;
    case PLACE_THOUSANDTHS:
      change = 0.001f;
    break;
    default:
    break;
  }
  
  setFactor(*factor+change,factor);
  char factorString[6] = {'\0'};
  
  // Can't sprintf a float so let's seperate it
  int d1 = *factor; // get the number to the left of the decimal
  int d2 = trunc(((*factor)-d1)*1000); // get the number to the right of the decimal
  sprintf(factorString,"%d.%.3d",d1,d2);
  lcd.setCursor(8,1);
  lcd.print(factorString);
  setFactorCursor(getPlace());
    
}

void decrementFactor(float * factor)
{
  float change = 0.000;
  uint8_t place = getPlace();
  switch(place)
  {
    case PLACE_ONES:
      change = -1.000f;
    break;
    case PLACE_TENTHS:
      change = -0.100f;
    break;
    case PLACE_HUNDREDTHS:
      change = -0.010f;
    break;
    case PLACE_THOUSANDTHS:
      change = -0.001f;
    break;
    default:
    break;
  }
  
  setFactor(*factor+change,factor);
  char factorString[6] = {'\0'};
  
  // Can't sprintf a float so let's seperate it
  int d1 = *factor; // get the number to the left of the decimal
  int d2 = trunc(((*factor)-d1)*1000); // get the number to the right of the decimal
  sprintf(factorString,"%d.%.3d",d1,d2);
  lcd.setCursor(8,1);
  lcd.print(factorString);
  setFactorCursor(getPlace());  
}

void setFactorCursor(uint8_t place)
{
  switch(place)
  {
    case PLACE_ONES:
      lcd.setCursor(8,1);
    break;
    case PLACE_TENTHS:
      lcd.setCursor(10,1);
    break;
    case PLACE_HUNDREDTHS:
      lcd.setCursor(11,1);
    break;
    case PLACE_THOUSANDTHS:
      lcd.setCursor(12,1);
    break;
    default:
    break;
  }
}

void movePlaceRight()
{
  setPlace(place+1);
}

void movePlaceLeft()
{
  setPlace(place-1);
}

void setPlace(const uint8_t p)
{
  if( (p>=PLACE_ONES) && (p<=PLACE_THOUSANDTHS))
  {
    place = p;
  }
}

uint8_t getPlace()
{
 return place; 
}

void resetPlace()
{
  place = PLACE_ONES;
}


