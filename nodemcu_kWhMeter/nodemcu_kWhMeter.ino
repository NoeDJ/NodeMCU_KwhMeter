// include the library code:
// #include <LiquidCrystal.h>#include <Key.h>
#include <Keypad.h>
#include <Keypad_I2C.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PZEM004Tv30.h>
#include <EEPROM.h>
#define I2CADDR 0x20 // Set the Address of the PCF8574

const byte ROWS = 4; // Set the number of Rows
const byte COLS = 4; // Set the number of Columns

// // Set the Key at Use (4x4)
// char keys[ROWS][COLS] = {
//     {'D', '#', '0', '*'},
//     {'C', '9', '8', '7'},
//     {'B', '6', '5', '4'},
//     {'A', '3', '2', '1'}};


// Set the Key at Use (4x4)
char keys [ROWS] [COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'}
};

// define active Pin (4x4)
byte rowPins[ROWS] = {0, 1, 2, 3}; // Connect to Keyboard Row Pin
byte colPins[COLS] = {4, 5, 6, 7}; // Connect to Pin column of keypad.

// makeKeymap (keys): Define Keymap
// rowPins:Set Pin to Keyboard Row
// colPins: Set Pin Column of Keypad
// ROWS: Set Number of Rows.
// COLS: Set the number of Columns
// I2CADDR: Set the Address for i2C
// PCF8574: Set the number IC
Keypad_I2C keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CADDR, PCF8574);

int eeAddress = 0; // eeprom eddress


int relayPinOut = 2;
int contact = 0;
int decimalValue = 0;
float inputInt;
String passWordString = "123456";
String inputString = "";
String inputString2 = "";
String inputString3 = "";

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long startMillis;
unsigned long currentMillis;
unsigned long period = 1000;
int getNextkey = 0;

PZEM004Tv30 pzem(14, 12);
float energy; //kWh
float power;  // Watt
// float token_spent; // satuan kWh
float token_remaining = 0.00; //  token Value satuan kWh
float token_remaining2;
float pricePerkWh = 0.00; // price per kWh

int xadd = 2;

// BUZZER
int pinBuzzer= 13;

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512);
  while (!Serial) ;
  //keypad setup
  Wire.begin();                   // Call the connection Wire
  keypad.begin(makeKeymap(keys)); // Call the connection

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Kamar : 1.A");
  //relaysetup
  pinMode(relayPinOut, OUTPUT);
  digitalWrite(relayPinOut, LOW);

  startMillis = millis();
  //Get the float data from the EEPROM at position 'eeAddress'     
  Serial.print("Read TOken from eeprom: ");
  String token_saveddata;
  token_saveddata = EEPROM_get(10);
  token_remaining = token_saveddata.toFloat();
//  EEPROM.get(eeAddress, token_remaining);
  Serial.println(token_remaining, 3); //This may print 'ovf, nan' if the data inside the EEPROM is not a valid float.
  pinMode(pinBuzzer, OUTPUT);

}

void loop()
{
  currentMillis = millis();
  // int xaddy = xadd += 1;
  // Serial.println(xaddy);
  energy = pzem.energy() ;
  // Serial.print("Read energy: ");  
  // Serial.print(energy);
  Serial.println(token_remaining);
  power = pzem.power();
  // float token_remaining2;
  int powerDecimalValue = getDecimalValue(power);
  int energyDecimalValue = getDecimalValue(power);
  int token_remainingDecimalValue = getDecimalValue(token_remaining);
  if (currentMillis - startMillis >= period)
  {
    if (!isnan(energy))
    {
      token_remaining2 = token_remaining - energy;      
      Serial.print("Read TOken: ");
      Serial.println(token_remaining2);
      if (token_remaining2 <= 0)
      {
        lcd.setCursor(0, 1);
        lcd.print("                     ");
        lcd.setCursor(0, 1);
        lcd.print("token habis");
      }      
      else if ((token_remaining2 <= 5) && (token_remaining2 > 0)) //harus disesuaikan lagi
      {
        digitalWrite(pinBuzzer, HIGH);
        delay(1000);
        digitalWrite(pinBuzzer, LOW);
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("               ");
        lcd.setCursor(0, 1);
        lcd.print("Token :");
        // lcd.print("kWh");
        lcd.setCursor(8, 1);
        lcd.print(token_remaining2, token_remainingDecimalValue);
        lcd.print("kWh");
        
      }
      else
      {
        lcd.setCursor(0, 1);
        lcd.print("               ");
        lcd.setCursor(0, 1);
        lcd.print("Token :");
        // lcd.print("kWh");
        lcd.setCursor(8, 1);
        lcd.print(token_remaining2, token_remainingDecimalValue);
        lcd.print("kWh");
      }
    }
    else
    {
      // Serial.println("Error reading energy");
      lcd.setCursor(0, 1);
      lcd.print("                     ");
      lcd.setCursor(0, 1);
      // lcd.print("token habis");
      lcd.print("Error! No Power");
    }
    startMillis = currentMillis;
  }
  if (token_remaining2 <= 0)
  {
    pzem.resetEnergy();
    digitalWrite(relayPinOut, HIGH);
  }
  else
  {
    digitalWrite(relayPinOut, LOW);
  }
  menu1();
  // Serial.println(token_remaining);
  // Serial.println(token_remaining2);
}

// ----------------------- keypad module ---------------------------------
void menu1()
{
  getNextkey = 0;
  char key = keypad.getKey(); // Create a variable named key of type char to hold the characters pressed
  if (key)
  {
    if (key == '*')
    {
      unsigned long Menu1startMillis = millis();
      Serial.println(key);
      inputString = "";        // clear input
      inputString2 = "PIN = "; // clear input
      period = 10000;
      lcd.setCursor(0, 1);
      lcd.print("                     ");
      lcd.setCursor(0, 1);
      lcd.print("PIN = ");

      while (getNextkey == 0)
      {
        delay(100);
        char key2 = keypad.getKey(); // Create a variable named key of type char to hold the characters pressed
        if (key2 >= '0' && key2 <= '9')
        {
          inputString += key2; // append new character to input string
          inputString2 += '*';
          lcd.setCursor(0, 1);
          lcd.print(inputString2);
          Serial.print(inputString);
          Serial.print("   ");
          Serial.println(inputString2);
        }
        else if (key2 == '*')
        {
          Serial.println(key2);
          Serial.println(inputString);
          if (inputString == passWordString)
          {
            menu2();
          }
          else
          {
            lcd.setCursor(0, 1);
            lcd.print("                     ");
            lcd.setCursor(0, 1);
            lcd.print("PIN Salah");
            delay(3000);
            break;
          }
          // Serial.println(key2);
          // break;
        }
        else if (millis() - Menu1startMillis >= period)
        {
          // Serial.print(Menu1startMillis);
          // Serial.print("     ");
          // Serial.println(millis());
          period = 1000;
          break;
        }
      }
    }
    else if (key == 'A')
    {
      token_remaining = 0.00;
      String tokendataeeprom = String(token_remaining, 3);
      EEPROM_put(10, tokendataeeprom);
//      EEPROM.put(eeAddress, token_remaining);
    }
    else if (key == 'B')
    {
      token_remaining2 = 10.00;
      String tokendataeeprom = String(token_remaining2, 3);
      token_remaining = token_remaining2;
      EEPROM_put(10, tokendataeeprom);
//      EEPROM.put(eeAddress, token_remaining);
    }
  }
}
void menu2()
{
  getNextkey = 1;
  unsigned long Menu2startMillis = millis();
  inputString = ""; // clear input
  inputString3 = "Jmlh kWh = ";
  period = 10000;
  lcd.setCursor(0, 1);
  lcd.print("                     ");
  lcd.setCursor(0, 1);
  lcd.print(inputString3);
  while (getNextkey == 1)
  {
    char key3 = keypad.getKey(); // Create a variable named key of type char to hold the characters pressed
    if (key3 >= '0' && key3 <= '9')
    {
      inputString += key3; // append new character to input string
      inputString3 += key3;
      Serial.println(inputInt);
      lcd.setCursor(0, 1);
      lcd.print("                     ");
      lcd.setCursor(0, 1);
      lcd.print(inputString3);
      // Serial.print(inputString);
    }
    else if (key3 == 'D')
    {
      inputString += "."; // append new character to input string
      inputString3 += ".";
      Serial.println(inputString);
      lcd.setCursor(0, 1);
      lcd.print("                     ");
      lcd.setCursor(0, 1);
      lcd.print(inputString3);
      // Serial.print(inputString);
    }

    else if (key3 == '#')
    {
      // Serial.println(key3);
      inputInt = inputString.toFloat();
      token_remaining = token_remaining + inputInt;
      token_remaining2 = token_remaining;
      
      String tokendataeeprom = String(token_remaining, 3);
      EEPROM_put(10, tokendataeeprom);
      
//      Serial.println(token_remaining2);
//      EEPROM.put(eeAddress, token_remaining);
      getNextkey == 0;
      period = 1000;
      break;
    }
    else if (millis() - Menu2startMillis >= period)
    {
      getNextkey == 0;
      period = 1000;
      break;
    }
  }
}
// ----------------------- end of keypad module ---------------------------------

float getDecimalValue(float power)
{
  // float power = pzem.power() * 1000;
  if (power < 10)
  {
    decimalValue = 3;
  }
  else if (10 <= power && power < 100) // x < 5 &&  x < 10
  {
    decimalValue = 2;
  }
  else if (100 <= power && power < 1000) // x < 5 &&  x < 10
  {
    decimalValue = 1;
  }
  else
  {
    decimalValue = 0;
  }
  return decimalValue;
}

void EEPROM_put(char add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}

String EEPROM_get(char add)
{
  int i;
  char data[10]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}
