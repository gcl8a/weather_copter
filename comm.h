#ifndef __COMM_H
#define __COMM_H

//#include <eeprom.h>
#include <RFM69.h>

//#define SerialUSB Serial

/////////////RFM69HCW radio
#define NETWORKID     155   // Must be the same for all nodes (0 to 255)
#define MYNODEID      1   // My node ID (0 to 255)
#define BASE_NODE     0   // My node ID (0 to 255)
#define FREQUENCY     RF69_915MHZ

// Create a library object for our RFM69HCW module:
//RFM69 radio(8, 3, false, digitalPinToInterrupt(3));
RFM69 radio(10, 8, false, digitalPinToInterrupt(8));

int InitRadio(void)
{
  SerialUSB.println(F("Initializing radio..."));

  // Initialize the RFM69HCW:
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower(); //needed for RFM69HCW

  SerialUSB.println("Success");

  return 1;
}

int WriteRadio(String str)
{
  //SerialUSB.print(str);

  int length = str.length();
  char sendBuffer[62];
  str.toCharArray(sendBuffer, 62); //we can send up to 255 characters; need to check if str is too long

  radio.send(BASE_NODE, sendBuffer, length > 62 ? 62 : length);
  radio.receiveDone(); //put us back into receive mode

  //return to receive mode???

  return 1;
}

String radioString;
bool CheckRadio(void)
{
  bool retVal = false;
  if(radio.receiveDone())
  {
    retVal = true;
    //uint8_t recLength = radio.DATALEN;

    radioString = "";
    for (byte i = 0; i < radio.DATALEN; i++)
    {
      if((char)radio.DATA[i] != '\n' && (char)radio.DATA[i] != '\r')
        radioString += (char)radio.DATA[i];
    }

    if (radio.ACKRequested())
    {
      radio.sendACK();
    }
  }

  return retVal;
}

void Report(const String& str)
{
  SerialUSB.print(millis());
  SerialUSB.print('\t');
  SerialUSB.println(str);
  //WriteRadio(str);
}

//void Interrupt1(void)
//{
//  
//}
//
//void Interrupt2(void)
//{
//  
//}
//
//const int INT1_PIN = 5; // INT1 pin to D5 - will be attached to gyro
//const int INT2_PIN = 6; // INT2 pin to D6 - attached to accel
//
//void configureLSM9DS1Interrupts(void)
//{
//  imu.configInt(XG_INT1, INT_DRDY_XL, INT_ACTIVE_LOW, INT_PUSH_PULL);
//  imu.configInt(XG_INT2, INT_DRDY_G , INT_ACTIVE_LOW, INT_PUSH_PULL);
//
//  attachInterrupt(digitalPinToInterrupt(INT1_PIN), Interrupt1, FALLING);
//  attachInterrupt(digitalPinToInterrupt(INT2_PIN), Interrupt2, FALLING);
//}


//    if(altimeter3115.CheckInt1())
//    {
//      altimeter3115.ReadDataAlt();
//      altimeter3115.toggleOneShot();
//    }


#endif
