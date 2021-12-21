#include "sd_card.h"
#include "communication.h"
#include "keypad.h"

char keypadBuffer[MAX_PASSWORD_LEN] = {0};
char keypadSDBuffer[MAX_PASSWORD_LEN] = {0};
char bluetoothBuffer[MAX_BT_SERIAL_LEN] = {0};
char bluetoothSDBuffer[MAX_PASSWORD_LEN] = {0};
//Keypad
int rowPins[NUMBER_OF_ROWS] = {16,22,32,33};
int columnPins[NUMBER_OF_COLUMNS] = {25,26,27,14};
Keypad keypad(rowPins,columnPins);
//Bluetooth
BluetoothSerial SerialBT;

//Functions
void ProcessBluetoothData(void);
void InputPassword(void);
void InputNewPassword(void);
void ChangePassword(void);
void InputPhoneNumber(void);
void AddPhoneNumber(void);

void setup() 
{
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1,-1,17); //for SIM800L
  SerialBT.begin("Smart Door");
  SD.begin(); //Uses pins 23,19,18 and 5
  SD_ReadFile(SD,"/kp.txt",keypadSDBuffer);
  Serial.print("Keypad password:");
  Serial.println(keypadSDBuffer);
  SD_ReadFile(SD,"/bt.txt",bluetoothSDBuffer);
  //SendSMS("+2349058041373","What's up my friend!!!!!!!!!!!!");
}

void loop() 
{
  //Bluetooth
  ProcessBluetoothData();
  //Keypad
  char key = keypad.GetChar();
  switch(key)
  {
    case '*':
      InputPassword();
      break;
    case 'C':
      ChangePassword();
      break;
    case 'A':
      AddPhoneNumber();
      break;
  }
}

void ProcessBluetoothData(void)
{
  GetBluetoothData(bluetoothBuffer);
  if(strcmp(bluetoothBuffer,"\0") != 0)
  {
    if(strcmp(bluetoothBuffer,bluetoothSDBuffer) == 0)
    {
      Serial.println("Password is correct");
      Serial.println("Door Open");
    }
    /*else if(strcmp(bluetoothBuffer,"Read") == 0)
    {
    }*/
    else
    {
      Serial.println("Incorrect password");
    }
    memset(bluetoothBuffer,'\0',MAX_BT_SERIAL_LEN);
  }  
}

void InputPassword(void)
{
  char countryCodePhoneNo[15] = {0};
  Serial.println("Entering password mode");
  keypad.GetData(keypadBuffer);
  if(strcmp(keypadBuffer,keypadSDBuffer) == 0)
  {
    Serial.println("Password is correct");
    Serial.println("Door Open");
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    int retry = keypad.RetryPassword(keypadBuffer,keypadSDBuffer);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("Password is now correct");
        Serial.println("Door Open");
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Intruder!!!!!");
        SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
        SendSMS(countryCodePhoneNo,"Intruder!!!!!");
        break;
    }
  }
}

void InputNewPassword(void)
{
  char newPassword[MAX_PASSWORD_LEN] = {0};
  keypad.GetData(keypadBuffer);
  strcpy(newPassword,keypadBuffer);
  Serial.println("Reenter the new password");
  keypad.GetData(keypadBuffer);
  if(strcmp(keypadBuffer,newPassword) == 0)
  {
    Serial.println("New password successfully created");
    strcpy(keypadSDBuffer,keypadBuffer);
    SD_WriteFile(SD,"/kp.txt",keypadBuffer);
  }
  else
  {
    Serial.println("Incorrect input, 2 attempts left");
    int retry = keypad.RetryPassword(keypadBuffer,newPassword);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("New password successfully created");
        strcpy(keypadSDBuffer,keypadBuffer);
        SD_WriteFile(SD,"/kp.txt",keypadBuffer);
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Unable to create new password");
        break;
    } 
  }  
}

void ChangePassword(void)
{
  char countryCodePhoneNo[15] = {0};
  Serial.println("Enter previous password");
  keypad.GetData(keypadBuffer);
  if(strcmp(keypadBuffer,keypadSDBuffer) == 0)
  {
    Serial.println("Correct, now enter new password");
    InputNewPassword();
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    int retry = keypad.RetryPassword(keypadBuffer,keypadSDBuffer);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("Correct, now enter new password");
        InputNewPassword();
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Intruder!!!!!");
        SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
        SendSMS(countryCodePhoneNo,"Intruder!!!!!");
        break;
    }  
  }
}

void InputPhoneNumber(void)
{
  Serial.println("Enter phone number");
  char phoneNumber[12] = {0};
  char countryCodePhoneNo[15] = {0};
  keypad.GetData(phoneNumber);
  GetCountryCodePhoneNo(countryCodePhoneNo,phoneNumber);
  Serial.print("Phone number:");
  Serial.println(countryCodePhoneNo);
  SD_WriteFile(SD,"/pn.txt",countryCodePhoneNo);  
}

void AddPhoneNumber(void)
{
  char countryCodePhoneNo[15] = {0};
  Serial.println("Enter the password");
  keypad.GetData(keypadBuffer);
  if(strcmp(keypadBuffer,keypadSDBuffer) == 0)
  {
    Serial.println("Password is correct");
    InputPhoneNumber();
  }
  else
  {
    Serial.println("Incorrect password, 2 attempts left");
    int retry = keypad.RetryPassword(keypadBuffer,keypadSDBuffer);
    switch(retry)
    {
      case PASSWORD_CORRECT:
        Serial.println("Password is correct");
        InputPhoneNumber();
        break;
      case PASSWORD_INCORRECT:
        Serial.println("Intruder!!!!!");
        SD_ReadFile(SD,"/pn.txt",countryCodePhoneNo);  
        SendSMS(countryCodePhoneNo,"Intruder!!!!!");
        break;
    }    
  }  
}
