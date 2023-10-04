bool USB_setup(uint32_t Baud_rate, uint16_t Wait_time_mS) {
  Serial.begin(Baud_rate);
  delay(Wait_time_mS);  //Give USB time to connect EDITME to shortest reliable time
  if (Serial) {
    // USB_flag = 1;  //Used so that when connected via USB, verbose status and error messages can be sent, but will still run if not connected
    Serial.println("Peltier control box connected");
    Serial.println("V.0.1 Oisin Shaw Sep 2023");

    return 1;
  } else {
    Serial.end();
    return 0;
  }
}

uint8_t Load_settings_uint8(uint8_t Default_value, char* Name, uint8_t Board_number, uint8_t Channel_number) {
  //Take_debugging_timestamps();
  uint8_t Key_Length = 0;
  char Key_to_find[KEY_MAX_LENGTH];
  //   if (Verbose_output) {
  //   Serial.print("Loading uint8_t begin");
  // }
  if (Board_number == 255) {
    Key_Length += sprintf(&Key_to_find[0], "%s", Name);
  } else {
    if (Channel_number == 255) {
      Key_Length += sprintf(&Key_to_find[0], "%s_%c", Name, Board_number + 65);
    } else {

      Key_Length += sprintf(&Key_to_find[0], "%s_%c%u", Name, Board_number + 65, Channel_number);
    }
  }
  //Take_debugging_timestamps();
  // Serial.print("Key built");
  Key_to_find[Key_Length] = '\0';
  uint8_t Return_value = SD_findInt(Key_to_find);  //EDITME what if T setpoint is 0?
                                                   //Take_debugging_timestamps();
  if (Return_value) {
    if (Verbose_output) {
      Serial.print(Key_to_find);
      Serial.print(" = ");
      Serial.println(Return_value);
    }
    //Take_debugging_timestamps();
    return Return_value;
  } else {
    if (Verbose_output) {
      Serial.print(Key_to_find);
      Serial.print(" not found, default used = ");
      Serial.println(Default_value);
    }
    //Take_debugging_timestamps();
    return Default_value;
  }
}

uint16_t Load_settings_uint16(uint16_t Default_value, char Name[KEY_MAX_LENGTH - 10], uint8_t Board_number, uint8_t Channel_number) {
  //Take_debugging_timestamps(); //0
  uint8_t Key_Length = 0;
  char Key_to_find[KEY_MAX_LENGTH];
  if (Board_number == 255) {
    Key_Length += sprintf(&Key_to_find[0], "%s", Name);
  } else {
    if (Channel_number == 255) {
      Key_Length += sprintf(&Key_to_find[0], "%s_%c", Name, Board_number + 65);
    } else {

      Key_Length += sprintf(&Key_to_find[0], "%s_%c%u", Name, Board_number + 65, Channel_number);
    }
  }
  //Take_debugging_timestamps(); //1
  Key_to_find[Key_Length] = '\0';
  uint16_t Return_value = SD_findInt(Key_to_find);
  //Take_debugging_timestamps();
  if (Return_value) {
    if (Verbose_output) {
      Serial.print(Key_to_find);
      Serial.print(" = ");
      Serial.println(Return_value);
    }
    //Take_debugging_timestamps();
    return Return_value;
  } else {
    if (Verbose_output) {
      Serial.print(Key_to_find);
      Serial.print(" not found, default used = ");
      Serial.println(Default_value);
    }
    //Take_debugging_timestamps();
    return Default_value;
  }
}

double Load_settings_double(double Default_value, char Name[KEY_MAX_LENGTH - 10], uint8_t Board_number, uint8_t Channel_number) {
  //Take_debugging_timestamps();
  uint8_t Key_Length = 0;
  char Key_to_find[KEY_MAX_LENGTH];
  if (Board_number == 255) {
    Key_Length += sprintf(&Key_to_find[0], "%s", Name);
  } else {
    if (Channel_number == 255) {
      Key_Length += sprintf(&Key_to_find[0], "%s_%c", Name, Board_number + 65);
    } else {

      Key_Length += sprintf(&Key_to_find[0], "%s_%c%u", Name, Board_number + 65, Channel_number);
    }
  }
  //Take_debugging_timestamps();
  Key_to_find[Key_Length] = '\0';
  double Return_value = SD_findFloat(Key_to_find);
  //Take_debugging_timestamps();
  if (Return_value) {
    if (Verbose_output) {
      Serial.print(Key_to_find);
      Serial.print(" = ");
      Serial.println(Return_value);
    }
    //Take_debugging_timestamps();
    return Return_value;
  } else {
    if (Verbose_output) {
      Serial.print(Key_to_find);
      Serial.print(" not found, default used = ");
      Serial.println(Default_value);
    }
    //Take_debugging_timestamps();
    return Default_value;
  }
}

double* Load_settings_string_pointer(double* Default_value, char Name[KEY_MAX_LENGTH - 10], uint8_t Board_number, uint8_t Channel_number) {
  //Take_debugging_timestamps();
  uint8_t Key_Length = 0;
  char Key_to_find[KEY_MAX_LENGTH];

  Key_Length += sprintf(&Key_to_find[0], "%s_%c%u", Name, Board_number + 65, Channel_number);

  //Take_debugging_timestamps();
  Key_to_find[Key_Length] = '\0';
  String Return_value = SD_findString(Key_to_find);
  //Take_debugging_timestamps();
  if (!Return_value.length()) {
    if (Verbose_output) {
      Serial.print(Key_to_find);
      Serial.println(" not found, default value used");
    }
    *PID_Input_map[Board_number][Channel_number][0] = Board_number + 65;
    *PID_Input_map[Board_number][Channel_number][1] = Channel_number + 48;
    //Take_debugging_timestamps();
    return Default_value;
  } else {
    uint8_t Board_pointer = Return_value[0] - 65;                                                                                             //ASCII A = uint8_t 65, convert the ASCII character to correct integer
    uint8_t Channel_pointer = Return_value[1] - 48;                                                                                           //ASCII  = uint8_t 48, convert the ASCII character to correct integer
    if ((-1 < Board_pointer) && (Board_pointer < MAX_Num_Driver_boards) && (-1 < Channel_pointer) && (Channel_pointer < Inputs_per_board)) {  //Check the returned values are within expected bounds
      if (Verbose_output) {
        Serial.print(Key_to_find);
        Serial.print(" = ");
        Serial.println(Return_value);
      }
      //EDITME insert return pointer code
      //Take_debugging_timestamps();
      *PID_Input_map[Board_number][Channel_number][0] = Board_pointer + 65;
      *PID_Input_map[Board_number][Channel_number][1] = Channel_pointer + 48;
      return NTC_T_readings[Board_pointer][Channel_pointer];
    } else {
      if (Verbose_output) {
        Serial.print(Key_to_find);
        Serial.println(" outside accepted range, default value used");
      }
      //Take_debugging_timestamps();
      *PID_Input_map[Board_number][Channel_number][0] = Board_number + 65;
      *PID_Input_map[Board_number][Channel_number][1] = Channel_number + 48;
      return Default_value;
    }
  }
}

void Take_debugging_timestamps() {
  Debugging_timestamps[Debugging_timestamp_position] = millis() - Debugging_timestamps[0];
  Debugging_timestamp_position += 1;
}

void Print_reset_debugging_timestamps() {
  Serial.print(Debugging_timestamp_position);
  Serial.println(",");
  for (uint8_t i = 0; i < Debugging_timestamp_position + 1; i++) {
    Serial.print(Debugging_timestamps[i]);
    Serial.println(",");
  }
  memset(&Debugging_timestamps[0], 0, 4 * (Debugging_timestamp_position + 1));
  Debugging_timestamp_position = 0;
}