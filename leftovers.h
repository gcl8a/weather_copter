
EEPROM_24LC256 eeprom(0x50);



////    else if(mode == CALIBRATING)
////    {
////      SerialUSB.println(F("Press return to calibrate accel/gyro"));
////      while(!SerialUSB.available()) {} //wait for keypress
////      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer
////
////      imu.CalibrateAccelerometerAndGyro(128);
////
////      SerialUSB.println(F("Press return to calibrate magnetometer"));
////      while(!SerialUSB.available()) {} //wait for keypress
////      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer
////
////      imu.CalibrateMagnetometer(512);
////    }
////


    else if(rx_byte == 'M') //calibrate imu
    {
      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer

      SerialUSB.println(F("Press return to calibrate accel/gyro"));
      while(!SerialUSB.available()) {} //wait for keypress
      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer

      imu.CalibrateAccelerometerAndGyroBasic(256);

      SerialUSB.println(F("Press S to save values"));
      while(!SerialUSB.available()) {} //wait for keypress
      if(SerialUSB.read() == 'S')
      {
        eeprom.StoreVector(imu.OffloadCalibrationDataAccel(), 0x100);
        delay(50);
        eeprom.StoreVector(imu.OffloadCalibrationDataGyro(), 0x200);    
      }
      
      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer

      SerialUSB.println(F("Press any key to calibrate magnetometer"));
      while(!SerialUSB.available()) {} //wait for keypress
      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer

      imu.CalibrateMagnetometer(128);

      SerialUSB.println(F("Press S to save values"));
      while(!SerialUSB.available()) {} //wait for keypress
      if(SerialUSB.read() == 'S')
      {
        eeprom.StoreVector(imu.OffloadCalibrationDataMag(), 0x300);
      }
      
      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer
    }

    else if(rx_byte == 'L') //load imu data
    {
      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer

      SerialUSB.println(F("Press L to load accel/gyro"));
      while(!SerialUSB.available()) {} //wait for keypress
      if(SerialUSB.read() == 'L')
      {
        float32vector accelData = eeprom.ReadVector(12, 0x100);
        imu.LoadCalibrationDataAccel(accelData);
        
        float32vector gyroData = eeprom.ReadVector(12, 0x200);
        imu.LoadCalibrationDataGyro(gyroData);
      }
      
      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer

      SerialUSB.println(F("Press L to load magnetometer values"));
      while(!SerialUSB.available()) {} //wait for keypress
      if(SerialUSB.read() == 'L')
      {
        float32vector magData = eeprom.ReadVector(12, 0x300);
        imu.LoadCalibrationDataMag(magData);
      }
      
      while(SerialUSB.available()) SerialUSB.read(); //clear out the buffer
    }
  }
  
//      WriteSD(filename, trisonica.GetReading().MakeDataString() + ',' 
//                      + gps.GetReading().MakeDataString() + ',' 
////                      + altimeter3115.MakeDataString() + ','
//                      + altimeter280.MakeDataString() + ','
//                      + imu.GetReading().MakeDataString() + '\n');
//      else if(rx_byte == '\n')
//      {
//        int alt = altString.toInt();
//        SerialUSB.print("Setting altitude to: ");
//        SerialUSB.println(alt);
//        altString = "";
//
//        float fOffset = altimeter3115.CalcOffset(alt);
//        int8_t offset = altimeter3115.SetOffset(fOffset);      
//
//        SerialUSB.print("Offset is: ");
//        SerialUSB.println(offset);
//      }
//
//      else altString += rx_byte;
//    }
