void Cmd_Unknown() {
  Serial.println(F("I don't know that command. Try another. "));
}

void Cmd_Reinitialise(CommandParameter& Parameters) {
  rp2040.idleOtherCore();
  // Check_available_memory(Parameters);

  //Free memory used on the heap
  for (uint8_t board = 0; board < Num_NTC_boards; board++) {
    delete NTC_CS_DIP[board];
    delete NTC_V_ref[board];
    for (uint8_t channel = 0; channel < Inputs_per_board; channel++) {
      delete NTC_RT0[board][channel];
      delete NTC_Beta[board][channel];
      delete NTC_R1[board][channel];
      delete NTC_R_inf[board][channel];
      for (uint8_t buffer_position = 0; buffer_position < Moving_average_window; buffer_position++) {
        delete NTC_Raw_Readings[board][channel][buffer_position];
      }
      delete NTC_Raw_average[board][channel];
      delete NTC_R_readings[board][channel];
      delete NTC_T_readings[board][channel];
    }
  }
  // Serial.println("NTC parameters deleted");

  for (uint8_t board = 0; board < Num_Driver_boards; board++) {
    delete Channel_output_flags[board];
    delete PWM_driver_address[board];
    delete PWM_driver[board];

    for (uint8_t driver = 0; driver < Drivers_per_board; driver++) {
      delete PID_Input_map[board][driver][0];
      delete PID_Input_map[board][driver][1];
      delete PID_Setpoint[board][driver];
      delete PID_Output[board][driver];
      delete PID_Output_Min[board][driver];
      delete PID_Output_Max[board][driver];
      delete PID_KP[board][driver];
      delete PID_KI[board][driver];
      delete PID_KD[board][driver];
      delete PID[board][driver];
    }
  }

  // Serial.println("Driver parameters deleted");

  // Check_available_memory(Parameters);

  rp2040.resumeOtherCore();
  rp2040.restartCore1();
  // rp2040.reboot();
  return;
}

void Cmd_Save_settings(CommandParameter& Parameters) {
  const char* Filename_from_CMD = Parameters.NextParameter();
  if (Verbose_output) {
    Serial.print("Filename: ");
    Serial.println(Filename_from_CMD);
  }
  if (Save_settings_to_file(Filename_from_CMD)) {
    if (Verbose_output) {
      Serial.println("Settings saved");
    }
  }
}

bool isValidLocation(const char* input, int& board, int& channel) {
  if (strcmp(input, "all") == 0) {
    board = -1;
    channel = -1;
    return true;  // "all" is a valid input
  } else if (strlen(input) == 1 && isalpha(input[0])) {
    board = input[0] - 'A';
    channel = -1;
    if (board < Num_NTC_boards) {  // assume driver boards = NTC boards
      return true;
    } else {
      return false;
    }
  } else if (strlen(input) == 2 && isalpha(input[0]) && isdigit(input[1])) {
    board = input[0] - 'A';
    channel = input[1] - '0';
    if (board < Num_NTC_boards && channel < Inputs_per_board) {  // assume driver boards = NTC boards
      return true;
    } else {
      return false;
    }
  } else {
    return false;  // Invalid input
  }
}



void interfaceVariable(CommandParameter& Parameters, bool set_status) {
  const char* location_cmd = Parameters.NextParameter();
  const char* variable_cmd = Parameters.NextParameter();

  int board_idx, channel_idx;
  if (isValidLocation(location_cmd, board_idx, channel_idx)) {
    // command sent for specific channel
    if (board_idx != -1 && channel_idx != -1) {
      // new variable
      if (strcmp(variable_cmd, "NTC_RT0") == 0) {
        if (set_status == true) {
          *NTC_RT0[board_idx][channel_idx] = Parameters.NextParameterAsInteger();
          Serial.print("NTC_RT0 updated to: ");
        } else if (set_status == false) {
          Serial.print("NTC_RT0 = ");
        }
        Serial.println(*NTC_RT0[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "NTC_Beta") == 0) {
        if (set_status == true) {
          *NTC_Beta[board_idx][channel_idx] = Parameters.NextParameterAsInteger();
          Serial.print("NTC_Beta updated to: ");
        } else if (set_status == false) {
          Serial.print("NTC_Beta = ");
        }
        Serial.println(*NTC_Beta[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_Setpoint") == 0) {
        if (set_status == true) {
          *PID_Setpoint[board_idx][channel_idx] = Parameters.NextParameterAsDouble();
          Serial.print("PID_Setpoint updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_Setpoint = ");
        }
        Serial.println(*PID_Setpoint[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_Output_Min") == 0) {
        if (set_status == true) {
          *PID_Output_Min[board_idx][channel_idx] = Parameters.NextParameterAsDouble();
          //*PID[board_idx][channel_idx]->setOutputRange(*PID_Output_Min[board_idx][channel_idx], *PID_Output_Max[board_idx][channel_idx]);
          Serial.print("PID_Output_Min updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_Output_Min = ");
        }
        Serial.println(*PID_Output_Min[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_Output_Max") == 0) {
        if (set_status == true) {
          *PID_Output_Max[board_idx][channel_idx] = Parameters.NextParameterAsDouble();
          //*PID[board_idx][channel_idx]->setOutputRange(*PID_Output_Min[board_idx][channel_idx], *PID_Output_Max[board_idx][channel_idx]);
          Serial.print("PID_Output_Max updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_Output_Max = ");
        }
        Serial.println(*PID_Output_Max[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_KP") == 0) {
        if (set_status == true) {
          *PID_KP[board_idx][channel_idx] = Parameters.NextParameterAsDouble();
          PID[board_idx][channel_idx]->setGains(*PID_KP[board_idx][channel_idx], *PID_KI[board_idx][channel_idx], *PID_KD[board_idx][channel_idx]);
          Serial.print("PID_KP updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_KP = ");
        }
        Serial.println(*PID_KP[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_KI") == 0) {
        if (set_status == true) {
          *PID_KI[board_idx][channel_idx] = Parameters.NextParameterAsDouble();
          PID[board_idx][channel_idx]->setGains(*PID_KP[board_idx][channel_idx], *PID_KI[board_idx][channel_idx], *PID_KD[board_idx][channel_idx]);
          Serial.print("PID_KI updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_KI = ");
        }
        Serial.println(*PID_KI[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_KD") == 0) {
        if (set_status == true) {
          *PID_KD[board_idx][channel_idx] = Parameters.NextParameterAsDouble();
          PID[board_idx][channel_idx]->setGains(*PID_KP[board_idx][channel_idx], *PID_KI[board_idx][channel_idx], *PID_KD[board_idx][channel_idx]);
          Serial.print("PID_KD updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_KD = ");
        }
        Serial.println(*PID_KD[board_idx][channel_idx]);
      }
      // unrecognised variable - error
      else {
        Serial.println(F("Variable does not exist or is not channel-specific."));
      }
      // command sent for specific board
    } else if (board_idx != -1 && channel_idx == -1) {
      // new variable
      if (strcmp(variable_cmd, "PWM_driver_address") == 0) {
        if (set_status == true) {
          *PWM_driver_address[board_idx] = Parameters.NextParameterAsInteger();
          Serial.print("PWM_driver_address updated to: ");
        } else if (set_status == false) {
          Serial.print("PWM_driver_address = ");
        }
        Serial.println(*PWM_driver_address[board_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "NTC_CS_DIP") == 0) {
        if (set_status == true) {
          *NTC_CS_DIP[board_idx] = Parameters.NextParameterAsInteger();
          Serial.print("NTC_CS_DIP updated to: ");
        } else if (set_status == false) {
          Serial.print("NTC_CS_DIP = ");
        }
        Serial.println(*NTC_CS_DIP[board_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "NTC_V_ref") == 0) {
        if (set_status == true) {
          *NTC_V_ref[board_idx] = Parameters.NextParameterAsDouble();
          Serial.print("NTC_V_ref updated to: ");
        } else if (set_status == false) {
          Serial.print("NTC_V_ref = ");
        }
        Serial.println(*NTC_V_ref[board_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "Channel_output_flags") == 0) {
        if (set_status == true) {
          Serial.println("Unable to start or stop channels with the Set command. Please use Start or Stop.");
        } else if (set_status == false) {
          Serial.print("Channel_output_flags = ");
          Serial.println(*Channel_output_flags[board_idx], BIN);
        }
      }
      // unrecognised variable - error
      else {
        Serial.println(F("Variable does not exist or is not board-specific."));
      }
      // command sent for 'all' boards and channels
    } else if (board_idx == -1 && channel_idx == -1) {
      // new variable
      if (strcmp(variable_cmd, "PID_Time_Step_mS") == 0) {
        if (set_status == true) {
          PID_Time_Step_mS = Parameters.NextParameterAsInteger();
          Serial.print("PID_Time_Step_mS updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_Time_Step_mS = ");
        }
        Serial.println(PID_Time_Step_mS);
      }
      // unrecognised variable - error
      else {
        Serial.println(F("Variable does not exist or is not global to all channels."));
      }
    }
  } else {
    Serial.println(F("Invalid location specified."));
  }
}

void Get_Variable(CommandParameter& Parameters) {
  interfaceVariable(Parameters, false);
}

void Set_Variable(CommandParameter& Parameters) {
  interfaceVariable(Parameters, true);
}

void printChannelCommand(int board, int channel, int command) {
  // Convert the first integer to the corresponding alphabet letter
  char letter = 'A' + board;
  String command_type;
  if (command == -1) {
    // Create the string message
    command_type = " stopped. ";
  } else if (command == 1) {
    command_type = " started. ";
  } else {
    command_type = ". ";
  }
  String message = "Channel " + String(letter) + String(channel) + command_type;

  // Print the message
  Serial.print(message);
  // printRunningChannels();
  // Serial.println();
}

void printRunningChannels() {
  Serial.print("Currently running channels: ");
  for (int i = 0; i < Num_Driver_boards; i++) {  // assume driver boards = PID boards
    uint8_t currentFlag = *Channel_output_flags[i];
    for (int bit = 0; bit < 8; bit++) {
      if (currentFlag & (1 << bit)) {
        char channelLetter = 'A' + i;
        Serial.print(String(channelLetter) + String(bit) + ", ");
      }
    }
  }
}

void channelSwitch(CommandParameter& Parameters, int direction) {
  const char* location_cmd = Parameters.NextParameter();
  const char* variable_cmd = Parameters.NextParameter();
  int board_idx, channel_idx;
  if (isValidLocation(location_cmd, board_idx, channel_idx)) {
    if (direction == -1) {
      *Channel_output_flags[board_idx] = *Channel_output_flags[board_idx] & ~(1 << channel_idx);
    } else if (direction == 1) {
      *Channel_output_flags[board_idx] = *Channel_output_flags[board_idx] | (1 << channel_idx);
    }
    printChannelCommand(board_idx, channel_idx, direction);
    printRunningChannels();
    Serial.println();
  } else {
    Serial.println(F("Invalid location specified."));
  }
}

void Start_Channel(CommandParameter& Parameters) {
  channelSwitch(Parameters, 1);
}

void Stop_Channel(CommandParameter& Parameters) {
  channelSwitch(Parameters, -1);
}

void Running_Channels(CommandParameter& Parameters) {
  printRunningChannels();
  Serial.println();
}

void Log_to_Serial_Set(CommandParameter& Parameters) {
  Log_to_Serial = Parameters.NextParameterAsInteger();
  if (Verbose_output) {
    Serial.print("Logging to Serial: ");
    Serial.println(Log_to_Serial);
  }
}

void Check_available_memory(CommandParameter& Parameters) {
  Serial.print("Memory Heap unallocated = ");
  Serial.println(rp2040.getFreeHeap());
}