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
    //EDITME write error throwing code
  } else {
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

  if (!logfile.open(Filename_array, O_RDWR | O_CREAT | O_TRUNC)) {
    if (USB_flag) {
      Serial.println("File open failed");
    }
    return 0;
  } else {
    return 1;
  }
}

bool SD_available(char *key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return value_length > 0;
}

int SD_findInt(char *key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Int(value_string, value_length);
}

float SD_findFloat(char *key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Float(value_string, value_length);
}

String SD_findString(char *key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2String(value_string, value_length);
}

int SD_findKey(char *key, char *value) {
  File32 configFile = SD.open(FILE_NAME);

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

  // check line by line
  while (configFile.available()) {
    int buffer_length = configFile.readBytesUntil(',', SD_buffer, 100);
    if (SD_buffer[buffer_length - 1] == '\r')
      buffer_length--;  // trim the \r

    if (buffer_length > (key_length + 1)) {                  // 1 is = character
      if (memcmp(SD_buffer, key_string, key_length) == 0) {  // equal
        if (SD_buffer[key_length] == '=') {
          value_length = buffer_length - key_length - 1;
          memcpy(value, SD_buffer + key_length + 1, value_length);
          break;
        }
      }
    }
  }

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

  File32 Settings_save_file;
  if (!Settings_save_file.open("test_save.txt", O_RDWR | O_CREAT | O_TRUNC)) {
    if (Verbose_output) {
      Serial.println("Settings save file open failed");
    }
    return 0;
  }

  char Data_to_file[200] = "\n";  //Initialise char array, beginning with a new line character
  Data_to_file[1] = ',';
  int j = 2;  //starts at 1 to account for newline chracter

    j += sprintf(&Data_to_file[j], "Num_NTC_boards=%u,", Num_NTC_boards);  //Append a reading, and then a delimeter
    j += sprintf(&Data_to_file[j], "Num_Driver_boards=%u,", Num_Driver_boards);  //Append a reading, and then a delimeter
    j += sprintf(&Data_to_file[j], "PID_Time_Step_mS=%u,", PID_Time_Step_mS);  //Append a reading, and then a delimeter

Serial.println(Data_to_file);
    logfile.write(Data_to_file,j);                       //Write the whole data string to the file
    logfile.sync();                                        //Save data to disc
    logfile.close();

  // //Initialise input board settings, loading from SD file where available and using defaults otherwise
  // for (uint8_t board = 0; board < Num_NTC_boards; board++) {
  //   NTC_CS[board] = new uint8_t(NTC_CS_pins[Load_settings_uint8(board, "NTC_CS\0", board)]);  //Assumes NTC input boards use DIP switch addresses of 0 upwards, no gaps and in order
  //   NTC_V_ref[board] = new double(Load_settings_double(NTC_V_ref_default, "NTC_V_ref\0", board));
  //   for (uint8_t channel = 0; channel < Inputs_per_board; channel++) {
  //     NTC_RT0[board][channel] = new uint16_t(Load_settings_uint16(NTC_RT0_default, "NTC_RT0\0", board, channel));
  //     NTC_Beta[board][channel] = new uint16_t(Load_settings_uint16(NTC_Beta_default, "NTC_Beta\0", board, channel));
  //     NTC_R1[board][channel] = new uint16_t(Load_settings_uint16(NTC_R1_default, "NTC_R1\0", board, channel));
  //     NTC_R_inf[board][channel] = new double(*NTC_RT0[board][channel] * exp((0.0 - *NTC_Beta[board][channel]) / NTC_T0));
  //     NTC_Raw_Readings[board][channel] = new uint16_t(0);
  //     NTC_R_readings[board][channel] = new double(0);
  //     NTC_T_readings[board][channel] = new double(0);
  //   }
  // }

  // //Initialise output board settings, loading from SD file where available and using defaults otherwise
  // for (uint8_t board = 0; board < Num_Driver_boards; board++) {
  //   for (uint8_t driver = 0; driver < Drivers_per_board; driver++) {
  //     PID_Input[board][driver] = Load_settings_string_pointer(NTC_T_readings[board][driver], "PID_Input\0", board, driver);  //Assumes outputs are controlled by corresponding inputs. A1 to A1, B3 to B3 etc
  //     PID_Setpoint[board][driver] = new double(Load_settings_double(NTC_V_ref_default, "PID_Setpoint\0", board, driver));
  //     PID_Output[board][driver] = new double();
  //     PID_Output_Min[board][driver] = new double(Load_settings_double(PID_Output_Min_default, "PID_Output_Min\0", board, driver));
  //     PID_Output_Max[board][driver] = new double(Load_settings_double(PID_Output_Max_default, "PID_Output_Max\0", board, driver));
  //     PID_KP[board][driver] = new double(Load_settings_double(0, "PID_KP\0", board, driver));  //Guessing gain values is shooting blind, jsut set to 0 until told otherwise and channel won't output anything
  //     PID_KI[board][driver] = new double(Load_settings_double(0, "PID_KI\0", board, driver));
  //     PID_KD[board][driver] = new double(Load_settings_double(0, "PID_KD\0", board, driver));
  //     PID[board][driver] = new AutoPID(PID_Input[board][driver], PID_Setpoint[board][driver], PID_Output[board][driver], *PID_Output_Min[board][driver], *PID_Output_Max[board][driver], *PID_KP[board][driver], *PID_KI[board][driver], *PID_KD[board][driver]);  //Initialise PID objects with appropriate arguments
  //   }
  // }

   return 1;
}