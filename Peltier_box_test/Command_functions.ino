void Cmd_Unknown() {
  Serial.println(F("I don't know that command. Try another. "));
}

void Cmd_Reinitialise(CommandParameter& Parameters) {
  rp2040.restartCore1();
}

// void Cmd_Save_settings(CommandParameter &Parameters) {
//  const char *Filename_from_CMD=Parameters.NextParameter();
//     // if (Verbose_output) {
//     //   Serial.print("Filename: ");
//     //   Serial.println(Filename_from_CMD);
//     // }
//   if (Save_settings_to_file(Filename_from_CMD)) {
//     if (Verbose_output) {
//       Serial.println("Settings saved");
//     }
//   }
// }

// bool isValidLocation(const char* input, int& board_idx, int& channel_idx) {
//   if (input == "all") {
//     board_idx = -1;
//     channel_idx = -1;
//     return 1; // "all" is a valid input
//   } else if (strlen(input) == 1 && isalpha(input[0])) {
//     board_idx = input[0];
//     channel_idx = -1;
//     return 1; // Single letter is valid
//   } else if (strlen(input) == 2 && isalpha(input[0]) && isdigit(input[1])) {
//     board_idx = input[0];
//     channel_idx = input[1];
//     return 1; // Letter-number pair is valid
//   } else {
//     return 0; // Invalid input
//   }
// }

// void Get_Variable(CommandParameter &Parameters) {
//   const char* location_cmd = Parameters.NextParameter();
//   const char* variable_cmd = Parameters.NextParameter();

//   if isValidLocation(location_cmd){
//     Serial.println("YESS");
//   }
// }

bool isValidLocation(const char* input, int& board_idx, int& channel_idx) {
  if (strcmp(input, "all") == 0) {
    board_idx = -1;
    channel_idx = -1;
    return true;  // "all" is a valid input
  } else if (strlen(input) == 1 && isalpha(input[0])) {
    board_idx = input[0] - 'A';
    channel_idx = -1;
    return true;  // Single letter is valid
  } else if (strlen(input) == 2 && isalpha(input[0]) && isdigit(input[1])) {
    board_idx = input[0] - 'A';
    channel_idx = input[1] - '0';  // Convert the digit character to an integer
    return true;                   // Letter-number pair is valid
  } else {
    return false;  // Invalid input
  }
}

void interface_Variable(CommandParameter &Parameters, bool set_status){
  const char* location_cmd = Parameters.NextParameter();
  const char* variable_cmd = Parameters.NextParameter();

  int board_idx, channel_idx;
  if (isValidLocation(location_cmd, board_idx, channel_idx)) {
    if (strcmp(variable_cmd, "NTC_RT0")==0) {
      if (set_status == true){
        NTC_RT0[board_idx][channel_idx] = new uint16_t(Parameters.NextParameterAsInteger());
        Serial.print("NTC_RT0 updated to: ");
      }
      else if (set_status == false){
        Serial.print("NTC_RT0 = ");
      }
      Serial.println(*NTC_RT0[board_idx][channel_idx]);
    } else {
      Serial.println(F("Variable does not exist or is not board-specific."));
    }
  }
  else {
    Serial.println(F("Invalid location specified."));
}
}

void Get_Variable(CommandParameter &Parameters) {
  interface_Variable(Parameters, false);
}

void Set_Variable(CommandParameter &Parameters) {
  interface_Variable(Parameters, true);
}


