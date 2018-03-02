/*
 * Weather copter code based on the SAMD21 (SF SAMD21 mini, Feather M0, etc.).
 * 
 * BE SURE TO CONFIRM THE PIN ASSIGNMENTS FOR THE SPI BUS, ETC.!!!!!!
 */

 //Version that uses the AT45 flash chip

#include <wiring_private.h> //needed for the pinPeripheral() function

#include <weather_copter.h>
#include "comm.h"

enum MODE {IDLE, READING, CALIBRATION_MODE, CMD_MODE, TRISONICA_MODE, FILE_MODE};

/*
 * setup serial for the Trisonica on SERCOM1: 
 * Feather pin 12 -> sercom 1:3 -> RX
 * Feather pin 11 -> sercom 1:0 -> TX
 */
//Uart triSerial (&sercom1, 12, 11, SERCOM_RX_PAD_3, UART_TX_PAD_0);
//
////and set up the IRQ handler
//void SERCOM1_Handler()
//{
//  triSerial.IrqHandler();
//}

/*
 * setup serial for the Trisonica on SERCOM1: 
 * SAMD21 mini pin 3 -> sercom 2:1* -> RX
 * SAMD21 mini pin 4 -> sercom 2:0* -> TX
 */
Uart triSerial (&sercom2, 3, 4, SERCOM_RX_PAD_1, UART_TX_PAD_0);

//and set up the IRQ handler
void SERCOM2_Handler()
{
  triSerial.IrqHandler();
}

//define the trisonica, using the newly created UART
Trisonica trisonica(&triSerial); 

//define the GPS, using the "built-in" Serial1
GPS_EM506 gps(&Serial1);

//MPL3115A2 altimeter3115(5);
BME280 altimeter280;
LSM9DS1 imu;

WeatherCopter weatherCopter;

//char filename[16]; //12 characters max

void setup() 
{
  SerialUSB.begin(115200);

  delay(2000); //allows us to get the Serial Monitor up, should we want

  while(!SerialUSB){}

  InitRadio();
  
  Report("Trisonica...");
  triSerial.begin(115200);

  //these are needed for the serial connection...don't know why they have to come after begin()
//  pinPeripheral(12, PIO_SERCOM);
//  pinPeripheral(11, PIO_SERCOM);

  pinPeripheral(3, PIO_SERCOM_ALT);
  pinPeripheral(4, PIO_SERCOM_ALT);

  Report("GPS...");

  //start the GPS serial
  Serial1.begin(9600);
  while(!Serial1){} //wait for connections
  gps.Init(); //doesn't really init; just sends data rate stuff

  Report(F("Checking for signal."));
  Report(F("Press return at any time to skip waiting for fix.\n"));
  while(!(gps.CheckSerial() & RMC) && !SerialUSB.available() && !radio.receiveDone()) 
  {
    trisonica.CheckSerial(); //"purge" the trisonica readings
  }

//  GPSDatum gpsDatum = gps.GetReading();
//  char filename[8];
//  sprintf(filename, "%02i%02i", gpsDatum.month, gpsDatum.day);

  //no longer need RMC
  gps.SetActiveNMEAStrings(GGA);

  Report("Altimeter...");
  //start the altimeter running...(could toggle on GPS, when active)
//  altimeter3115.toggleOneShot();

  BME280Settings settings; //use the defaults
  altimeter280.Init(settings);

  Report("IMU...");
  IMUSettings imuSettings; //use defaults
  imu.Init(imuSettings);

//  SerialUSB.println("Testing");
//  IMUTest();

  weatherCopter.Init();
  weatherCopter.ListStores(true);

  Report(F("Done."));
}

int mode = CMD_MODE;

String cmdString; //for adjusting altitude

void loop() 
{
  if(SerialUSB.available())
  {
    char ch = SerialUSB.read();
    if(ch != '\n' && ch != '\r')
      cmdString += ch;
      
    else if(ch == '\n')
    {
      ProcessCmdString(cmdString);
      cmdString = "";
    }
  }

  if(CheckRadio())
  {
      ProcessCmdString(radioString);
      radioString = "";
  }

  if(mode == READING)
  {
    uint8_t nmeaType = gps.CheckSerial();
    if(nmeaType & GGA) //only need GGA for the weather copter
    {
      //send some basic data to the ground station
      Report(//altimeter3115.MakeShortDataString() + ',' 
                 altimeter280.MakeShortDataString() + ','
                  + trisonica.GetReading().MakeShortDataString());// + '\n');
      weatherCopter.AddGPSAltDump(gps.GetReading(), altimeter280.GetReading());
    }
  
    if(trisonica.CheckSerial()) 
    {
      //trisonica keys a new wind record
//      String dataStr =  gps.MakeDataString() + ',' 
//                      + trisonica.GetReading().MakeDataString() + ','
//                      + altimeter280.MakeDataString() + ','
//                      + imu.CalcRPY().MakeDataString();// + '\n';

//      SerialUSB.println(dataStr);
      //weatherCopter.WriteSDBlock(dataStr);
      weatherCopter.AddWindAndIMUDump(trisonica.GetReading(), imu.CalcRPY());
    }

    if(imu.IsAvailableAccelAndGyro())
    {
      imu.ProcessReadings();
    }

    if(altimeter280.CheckForNewDatum())
    {
      altimeter280.ReadDatum();
      altimeter280.ForceReading();
    }
  }

  else if(mode == TRISONICA_MODE)
  {
    if (triSerial.available())
      SerialUSB.write(triSerial.read());
  }
}

int ProcessCmdString(const String& cmdString)
{
  SerialUSB.println(cmdString);

  if(cmdString.length() == 0)
  {
    SerialUSB.println(F("Invalid command string (00)."));
    return 0;
  }

  char cmd = cmdString[0];

  switch(mode)
  {
    case CMD_MODE:
      switch(cmd)
      {
        case 'X':
          mode = READING;
          SerialUSB.println(F("Entering reading mode."));
          //weatherCopter.OpenSDBlockFile();
          break;
  
        case 'M': //'M' for iMu
          //calibrate
          SerialUSB.println(F("Entering calibration mode."));
          mode = CALIBRATION_MODE;
          break;
  
        case 'T':
          SerialUSB.println(F("Entering Trisonica mode."));
          mode = TRISONICA_MODE;
          triSerial.write(0x03);
          break;
  
        case 'F':
          SerialUSB.println(F("Entering file mode."));
          mode = FILE_MODE;
          weatherCopter.ListStores();
          break;
      }
      break;

    case TRISONICA_MODE:
      if(cmd == 'X') 
      {
        mode = CMD_MODE;
      }
      else triSerial.write(cmd); 
      break;

    case CALIBRATION_MODE:
      //calibration options
      //load, save, calibrate, clear, sample, also calibrate altitude
      //to be done someday
      break;

    case FILE_MODE:
      switch(cmd)
      {
        case 'X':
          SerialUSB.println("Entering command mode");
          mode = CMD_MODE;
          break;
        case 'O':
          weatherCopter.OpenStore(cmdString.substring(1).toInt());
          weatherCopter.ListStores();
          break;
        case 'S': 
          weatherCopter.SplashStore(cmdString.substring(1).toInt());
          break;
        case 'L':
          weatherCopter.ListStores();
          break;  
        case 'E':
          weatherCopter.EraseStore(cmdString.substring(1).toInt());
          weatherCopter.ListStores();
          break;
        case 'C':
          weatherCopter.CloseStore();
          weatherCopter.ListStores();
          break;
        case 'W':
          weatherCopter.SaveStoreToSD(cmdString.substring(1).toInt());
          weatherCopter.ListStores();
          break;
        case 'R':
          weatherCopter.ListStores(true);
          break;
        default:
          SerialUSB.println("Invalid command");
      }
      break; //end FILE_MODE

    case READING:
      if(cmd == 'C')
      {
        weatherCopter.CloseStore();
        mode = CMD_MODE;
        SerialUSB.println("Entering command mode");
      }
      break;

    default:
      SerialUSB.println("Oops! Unhandled mode!");
  }

  return mode;
}

