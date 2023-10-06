void Initialise_SD(uint8_t SCK, uint8_t COPI, uint8_t CIPO, uint8_t CS) {
  SPI.setSCK(SCK);  //Set the correct SPI pins for the SD SPI
  SPI.setTX(COPI);
  SPI.setRX(CIPO);
  SPI.setCS(CS);
  if (USB_flag) {
    Serial.print("Initializing SD card...");
  }

  if (!SD.begin(SD_CONFIG)) {
    if (USB_flag) {
      Serial.println("initialization failed!");
    }

    // if (!SD.begin(CS)) {
    //   Serial.println("initialization failed!");
    return;
  }
  // SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));  //Set the SPI settings so the ADC's work
  // SPI.endTransaction();
  //EDITME write error throwing code
  // }
  else {
    SD_boot_flag = 1;
    if (USB_flag) {
      Serial.println("initialization done.");
    }
  }
}

bool Create_logfile(DateTime Log_time, char *Filename_array) {
  String filename_string = String(Log_time.year(), DEC) + '_';
  if (Log_time.month() < 10) {
    filename_string += '0' + String(Log_time.month(), DEC) + "_";
  } else {
    filename_string += String(Log_time.month(), DEC) + "_";
  }
  if (Log_time.day() < 10) {
    filename_string += '0' + String(Log_time.day(), DEC) + "_";
  } else {
    filename_string += String(Log_time.day(), DEC) + "_";
  }
  if (Log_time.hour() < 10) {
    filename_string += '0' + String(Log_time.hour(), DEC) + "_";
  } else {
    filename_string += String(Log_time.hour(), DEC) + "_";
  }
  if (Log_time.minute() < 10) {
    filename_string += '0' + String(Log_time.minute(), DEC) + "_";
  } else {
    filename_string += String(Log_time.minute(), DEC) + "_";
  }
  if (Log_time.second() < 10) {
    filename_string += '0' + String(Log_time.second(), DEC);
  } else {
    filename_string += String(Log_time.second(), DEC);
  }
  filename_string += ".txt";
  filename_string.toCharArray(Filename_array, 26);
  if (USB_flag) {
    Serial.print("Filename generated: ");
    Serial.println(Filename_array);
  }

  // if (!logfile.open(Filename_array, O_RDWR | O_CREAT | O_TRUNC)) {
  //   if (USB_flag) {
  //     Serial.println("File open failed");
  //   }
  //   return 0;
  // } else {
  //   return 1;
  // }

  // logfile = SD.open(Filename_array, FILE_WRITE);
  // if (!myFile) {
  //   if (USB_flag) {
  //     Serial.println("File open failed");
  //   }
  //   return 0;
  // } else {
  //   return 1;
  // }
  // myFile.close();
  return 1;
}

bool SD_available(char *key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return value_length > 0;
}

int SD_findInt(char *key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  //Take_debugging_timestamps();
  return HELPER_ascii2Int(value_string, value_length);
}

float SD_findFloat(char *key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  //Take_debugging_timestamps();
  return HELPER_ascii2Float(value_string, value_length);
}

String SD_findString(char *key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  //Take_debugging_timestamps();
  return HELPER_ascii2String(value_string, value_length);
}

int SD_findKey(char *key, char *value) {
  File32 configFile = SD.open(FILE_NAME);
  //Take_debugging_timestamps();//2

  if (!configFile) {
    Serial.print(F("SD Card: error on opening file "));
    Serial.println(FILE_NAME);
    return 0;
  }

  char key_string[KEY_MAX_LENGTH];
  char SD_buffer[KEY_MAX_LENGTH + VALUE_MAX_LENGTH + 1];  // 1 is = character
  int key_length = 0;
  int value_length = 0;

  // Flash string to string
  PGM_P keyPointer;
  keyPointer = reinterpret_cast<PGM_P>(key);
  byte ch;
  do {
    ch = pgm_read_byte(keyPointer++);
    if (ch != 0)
      key_string[key_length++] = ch;
  } while (ch != 0);
  //Take_debugging_timestamps();
  // check line by line
  while (configFile.available()) {
    int buffer_length = configFile.readBytesUntil(',', SD_buffer, 100);
    // if (SD_buffer[buffer_length - 1] == '\r')
    //   buffer_length--;  // trim the \r

    if (buffer_length > (key_length + 1)) {                  // 1 is = character
      if (memcmp(SD_buffer, key_string, key_length) == 0) {  // equal
        if (SD_buffer[key_length] == '=') {
          value_length = buffer_length - key_length - 1;
          memcpy(value, SD_buffer + key_length + 1, value_length);
          break;
        }
      }
    }
    //Take_debugging_timestamps();
  }
  //Take_debugging_timestamps();//3

  configFile.close();  // close the file
  return value_length;
}

int HELPER_ascii2Int(char *ascii, int length) {
  int sign = 1;
  int number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

float HELPER_ascii2Float(char *ascii, int length) {
  int sign = 1;
  int decimalPlace = 0;
  float number = 0;
  float decimal = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c == '.')
        decimalPlace = 1;
      else if (c >= '0' && c <= '9') {
        if (!decimalPlace)
          number = number * 10 + (c - '0');
        else {
          decimal += ((float)(c - '0') / pow(10.0, decimalPlace));
          decimalPlace++;
        }
      }
    }
  }

  return (number + decimal) * sign;
}

String HELPER_ascii2String(char *ascii, int length) {
  String str;
  str.reserve(length);
  str = "";

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str += String(c);
  }

  return str;
}

bool Save_settings_to_file(const char *Filename) {

rp2040.idleOtherCore();
  Serial.println("File open begin");
  File32 Settings_save_file;//= SD.open("test.txt", FILE_WRITE);

  // if (!Settings_save_file) {
  //   Serial.print(F("SD Card: error on opening file "));
  //   Serial.println(Filename);
  //   return 0;
  // }


  if (!Settings_save_file.open(Filename, O_RDWR | O_CREAT | O_TRUNC)) {
    if (Verbose_output) {
      Serial.println("Settings save file open failed");
    }
    rp2040.resumeOtherCore();
    return 0;
  }

  Serial.println("File opened");

  // Settings_save_file.truncate(0);

  // Serial.println("File truncated");

  char Data_to_file[200] = ",";  //Initialise char array, beginning with a comma
  // Data_to_file[1] = ',';
  int j = 1;  //starts at 1 to account for newline chracter

  j += sprintf(&Data_to_file[j], "Num_NTC_boards=%u,", Num_NTC_boards);        //Append a reading, and then a delimeter
  j += sprintf(&Data_to_file[j], "Num_Driver_boards=%u,", Num_Driver_boards);  //Append a reading, and then a delimeter
  j += sprintf(&Data_to_file[j], "PID_Time_Step_mS=%u,", PID_Time_Step_mS);    //Append a reading, and then a delimeter

  // Serial.println(Data_to_file);
  Settings_save_file.write(&Data_to_file, j);  //Write the whole data string to the file
  Settings_save_file.sync();                   //Save data to disc

  // //Save input board settings to SD
  for (uint8_t board = 0; board < Num_NTC_boards; board++) {
    memset(&Data_to_file[0], 0, 200);
    Data_to_file[0] = '\n';
    Data_to_file[1] = ',';
    j = 2;
    j += sprintf(&Data_to_file[j], "NTC_CS_%c=%u,", board + 65, *NTC_CS_DIP[board]);    //Append a reading, and then a delimeter
    j += sprintf(&Data_to_file[j], "NTC_V_ref_%c=%f,", board + 65, *NTC_V_ref[board]);  //Append a reading, and then a delimeter //EDITME check if this uses DIP address or pin number
    Settings_save_file.write(&Data_to_file, j);                                         //Write the whole data string to the file
    Settings_save_file.sync();                                                          //Save data to disc
    for (uint8_t channel = 0; channel < Inputs_per_board; channel++) {
      memset(&Data_to_file[0], 0, 200);
      Data_to_file[0] = '\n';
      Data_to_file[1] = ',';
      j = 2;

      j += sprintf(&Data_to_file[j], "NTC_RT0_%c%u=%u,", board + 65, channel, *NTC_RT0[board][channel]);    //Append a reading, and then a delimeter
      j += sprintf(&Data_to_file[j], "NTC_Beta_%c%u=%u,", board + 65, channel, *NTC_Beta[board][channel]);  //Append a reading, and then a delimeter
      j += sprintf(&Data_to_file[j], "NTC_R1_%c%u=%u,", board + 65, channel, *NTC_R1[board][channel]);      //Append a reading, and then a delimeter

      Settings_save_file.write(&Data_to_file, j);  //Write the whole data string to the file
      Settings_save_file.sync();                   //Save data to disc
    }
  }

  //Save output board settingsto SD
  for (uint8_t board = 0; board < Num_Driver_boards; board++) {
    memset(&Data_to_file[0], 0, 200);
    Data_to_file[0] = '\n';
    Data_to_file[1] = ',';
    j = 2;

    j += sprintf(&Data_to_file[j], "Channel_autostart_flags_%c=%u,", board + 65, *Channel_output_flags[board]);  //Append a reading, and then a delimeter
    j += sprintf(&Data_to_file[j], "PWM_driver_address_%c=%u,", board + 65, *PWM_driver_address[board]);         //Append a reading, and then a delimeter

    Settings_save_file.write(&Data_to_file, j);  //Write the whole data string to the file
    Settings_save_file.sync();                   //Save data to disc

    for (uint8_t driver = 0; driver < Drivers_per_board; driver++) {

      memset(&Data_to_file[0], 0, 200);
      Data_to_file[0] = '\n';
      Data_to_file[1] = ',';
      j = 2;

      j += sprintf(&Data_to_file[j], "PID_Input_%c%u=%c%c,", board + 65, driver, *PID_Input_map[board][driver][0], *PID_Input_map[board][driver][1]);  //Append a reading, and then a delimeter
      j += sprintf(&Data_to_file[j], "PID_Setpoint_%c%u=%f,", board + 65, driver, *PID_Setpoint[board][driver]);                                       //Append a reading, and then a delimeter
      j += sprintf(&Data_to_file[j], "PID_Output_Min_%c%u=%f,", board + 65, driver, *PID_Output_Min[board][driver]);                                   //Append a reading, and then a delimeter
      j += sprintf(&Data_to_file[j], "PID_Output_Max_%c%u=%f,", board + 65, driver, *PID_Output_Max[board][driver]);                                   //Append a reading, and then a delimeter
      j += sprintf(&Data_to_file[j], "PID_KP_%c%u=%f,", board + 65, driver, *PID_KP[board][driver]);                                                   //Append a reading, and then a delimeter
      j += sprintf(&Data_to_file[j], "PID_KI_%c%u=%f,", board + 65, driver, *PID_KI[board][driver]);                                                   //Append a reading, and then a delimeter
      j += sprintf(&Data_to_file[j], "PID_KD_%c%u=%f,", board + 65, driver, *PID_KD[board][driver]);                                                   //Append a reading, and then a delimeter


      Settings_save_file.write(&Data_to_file, j);  //Write the whole data string to the file
      Settings_save_file.sync();                   //Save data to disc
    }
  }

  Settings_save_file.close();
rp2040.resumeOtherCore();
  return 1;
}