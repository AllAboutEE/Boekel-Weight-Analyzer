#include "screen_actions.h"
#include <LiquidCrystal.h>

extern LiquidCrystal lcd;
static uint8_t flagsScreenAction = FLAG_NO_SCREEN_ACTION;

static uint8_t place = PLACE_ONES;


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


// enter factor functions

void setFactor(float value, float *factor)
{

  *factor = roundf( (*factor) * 1000)/1000;
  
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

void incrementFactor(float * f)
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
  
  setFactor(*f+change,f);
  char factorString[6] = {'\0'};
  
  // Can't sprintf a float so let's seperate it
  int d1 = *f; // get the number to the left of the decimal
  int d2 = trunc(((*f)-d1)*1000); // get the number to the right of the decimal
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


void updateCRMainScreen(float reading)
{
 lcd.setCursor(4,1);
 lcd.print(reading);
}

void updateCRMainScreen(const char * s)
{
 lcd.setCursor(4,1);
 lcd.print(s);
}

void updateFirstCalibrationRemaningTime(char * timeRemaining)
{
  lcd.setCursor(12,1);
  lcd.print(timeRemaining);
}
