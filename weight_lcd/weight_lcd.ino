#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "HX711.h"

//Load cell
#define DOUT  3
#define CLK  2

long val = 0;
float count = 0;
float weight = 0;
float zero_weight = 158145.0f;

HX711 cell(DOUT, CLK);

//LCD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int ButtonVoltage = 0;


const int keypad_pin = A0;
int keypad_value = 0;
int keypad_value_old = 0;

char btn_push;

byte mainMenuPage = 1;
byte mainMenuPageOld = 1;
byte mainMenuTotal = 4;

// EEPROM

int eeAddress = 1;   //Location we want the data to be put.
int des_Address = 5;//eeAddress + sizeof(float); // = 4
int margin_Address = 9;//eeAddress + 2*sizeof(float); // = 8

float des_weight = 100.0f;
float margin = 10.0f;

//OUTPUT
const int output_pin = A1;

void setup() {
  Serial.begin(9600);
  delay(10);
  /*
  //write values to EEPROM
  EEPROM.put(des_Address, des_weight);
  Serial.print("des_weight address: "); Serial.println(des_Address);
  EEPROM.put(margin_Address, margin);
  Serial.print("margin address: "); Serial.println(margin_Address);
  EEPROM.put(eeAddress, zero_weight); //One simple call, with the address first and the object second.
  Serial.print("zero_weight address: "); Serial.println(eeAddress);
  */
  //read values from EEPROM
  EEPROM.get(eeAddress, zero_weight); //One simple call, with the address first and the object second.
  Serial.print("zero_weight value: "); Serial.println(zero_weight);
  EEPROM.get(des_Address, des_weight);
  Serial.print("des_weight value: "); Serial.println(des_weight);
  EEPROM.get(margin_Address, margin);
  Serial.print("margin value: "); Serial.println(margin);
  
  
  //LCD Menu
  lcd.begin(16,2);  //Initialize a 2x16 type LCD
  MainMenuDisplay();
  //delay(1000);

  //OUTPUT
  pinMode(output_pin, OUTPUT);
  digitalWrite(output_pin, LOW);
}

void loop() {
    btn_push = ReadKeypad();   
    MainMenuBtn();
    if(btn_push == 'S')//enter selected menu
    {
        WaitBtnRelease();
        switch (mainMenuPage)
        {
            case 1:  MenuA();   break;
            case 2:  MenuB();   break;
            case 3:  MenuC();   break;
            case 4:  MenuD();   break;
        }

          MainMenuDisplay();
          WaitBtnRelease();
    }
    
    delay(10);
}

float measure_loadcell(){
  count = count + 1;
  //Serial.print("Reading: ");
  val = 0.5 * val    +   0.5 * cell.read();
  weight = (val - zero_weight)/-20432.0f * 100;
  //Serial.print(weight);
  //Serial.println(" grams");
  //zero 158145
  //100g -20432
  return weight;
}

float zero_loadcell(){
  for (count = 1; count<200; count++){
  zero_weight = ((count-1)/count) * zero_weight  +  (1/count) * cell.read();
  Serial.print(count);
  Serial.print(" ");
  Serial.println(zero_weight);
  }
  return zero_weight;
}

void MenuA()
{  
    lcd.clear();
    lcd.setCursor(0,0);
    while(ReadKeypad()!= 'L')
    {
      lcd.clear();
      lcd.setCursor(0,0);
      weight = measure_loadcell();
      lcd.print(weight); lcd.print(" g");

      lcd.setCursor(0,1);
      lcd.print(des_weight);
      lcd.print(" g");

      lcd.setCursor(8,1);
      lcd.print("+-");
      lcd.print(margin);
      lcd.print("g");

      if (weight > (des_weight+margin)) {
        //overweight
        lcd.setCursor(8,0);
        lcd.print("OVER");
        Serial.println("overweight");
        digitalWrite(output_pin, HIGH);
      }

      else if (weight < (des_weight-margin)) {
        //underweight
        lcd.setCursor(8,0);
        lcd.print("UNDER");
        Serial.println("underweight");
        digitalWrite(output_pin, HIGH);
      }

      else {
        Serial.println("in range");
        digitalWrite(output_pin, LOW);
      }
      
      if (ReadKeypad() == 'S'){ //SAVE
          EEPROM.put(des_Address, des_weight);
          Serial.print("updated des_weight: "); Serial.println(des_weight);
          lcd.setCursor(11,0);
          lcd.print("SAVED");
        }
      
      delay(100);
    }
}


void MenuB()
{  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Zeroing");
    while(ReadKeypad()!= 'L')
    {
        zero_weight = zero_loadcell();
        lcd.setCursor(1,0);
        lcd.print("Finished");
        //update in EEPROM
        EEPROM.put(eeAddress, zero_weight);
        Serial.print("updated zero_weight: "); Serial.println(zero_weight);
        break;
    }
}


void MenuC()
{  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set weight:");
    
    while(ReadKeypad()!= 'L')
    {
        lcd.setCursor(0,1);
        lcd.print(des_weight);
        lcd.print(" g");
        //Adjusting

        if (ReadKeypad() == 'U'){ //up 10g
          des_weight += 10.0f;
          if (des_weight < 0.0f) des_weight = 0.0f;
          Serial.print("changed des_weight: "); Serial.println(des_weight);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Set weight:");
        }
        
        if (ReadKeypad() == 'D'){ //Down 10g
          des_weight -= 10.0f;
          if (des_weight < 0.0f) des_weight = 0.0f;
          Serial.print("changed des_weight: "); Serial.println(des_weight);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Set weight:");
        }

        if (ReadKeypad() == 'S'){ //SAVE
          EEPROM.put(des_Address, des_weight);
          Serial.print("updated des_weight: "); Serial.println(des_weight);
          lcd.setCursor(11,0);
          lcd.print("SAVED");
        }
       delay(100);
    }
}
void MenuD()
{  
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set desired");
    lcd.setCursor(0,1);
    lcd.print("margins:+-");
    while(ReadKeypad()!= 'L')
    {
        lcd.setCursor(10,1);
        lcd.print(margin);
        lcd.print("g");
        //Adjusting

        if (ReadKeypad() == 'U'){ //up 1g
          margin += 1.0f;
          if (margin < 0.0f) margin = 0.0f;
          Serial.print("changed margin: "); Serial.println(margin);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Set desired");
          lcd.setCursor(0,1);
          lcd.print("margins:+-");
        }
        
        if (ReadKeypad() == 'D'){ //Down 10g
          margin -= 1.0f;
          if (margin < 0.0f) margin = 0.0f;
          Serial.print("changed margin: "); Serial.println(margin);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Set desired");
          lcd.setCursor(0,1);
          lcd.print("margins:+-");
        }

        if (ReadKeypad() == 'S'){ //SAVE
          EEPROM.put(margin_Address, margin);
          Serial.print("updated margin: "); Serial.println(margin);
          lcd.setCursor(11,0);
          lcd.print("SAVED");
        }
       delay(100);
       
    }
}













void MainMenuDisplay()
{
    lcd.clear();
    lcd.setCursor(0,0);
    switch (mainMenuPage)
    {
        case 1:
          lcd.print("1. Measure");
          break;
        case 2:
          lcd.print("2. Zero scale");
          break;
        case 3:
          lcd.print("3. Set weight");
          break;
        case 4:
          lcd.print("4. Set margin");
          break;
    }
}

void MainMenuBtn()
{
    WaitBtnRelease();
    if(btn_push == 'D')
    {
        mainMenuPage++;
        if(mainMenuPage > mainMenuTotal)
          mainMenuPage = 1;
    }
    else if(btn_push == 'U')
    {
        mainMenuPage--;
        if(mainMenuPage == 0)
          mainMenuPage = mainMenuTotal;    
    }
    
    if(mainMenuPage != mainMenuPageOld) //only update display when page change
    {
        MainMenuDisplay();
        mainMenuPageOld = mainMenuPage;
    }
}

char ReadKeypad()
{
  /* Keypad button analog Value
  no button pressed 1023
  select  741
  left    503
  down    326
  up      142
  right   0 
  */
  keypad_value = analogRead(keypad_pin);
  
  if(keypad_value < 100)
    return 'R';
  else if(keypad_value < 200)
    return 'U';
  else if(keypad_value < 400)
    return 'D';
  else if(keypad_value < 600)
    return 'L';
  else if(keypad_value < 800)
    return 'S';
  else 
    return 'N';
}

void WaitBtnRelease()
{
    while( analogRead(keypad_pin) < 800){} 
}
