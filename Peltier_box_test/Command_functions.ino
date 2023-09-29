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

bool isValidLocation(const char* input, int& board_idx, int& channel_idx) {
  if (strcmp(input, "all") == 0) {
    board_idx = -1;
    channel_idx = -1;
    return true;  // "all" is a valid input
  } else if (strlen(input) == 1 && isalpha(input[0])) {
    board_idx = input[0] - 'A';
    channel_idx = -1;
    if (board_idx < Num_NTC_boards) {  // assume driver boards = NTC boards
      return true;
    } else {
      return false;
    }
  } else if (strlen(input) == 2 && isalpha(input[0]) && isdigit(input[1])) {
    board_idx = input[0] - 'A';
    channel_idx = input[1] - '0';
    if (board_idx < Num_NTC_boards && channel_idx < Inputs_per_board) {  // assume driver boards = NTC boards
      return true;
    } else {
      return false;
    }
  } else {
    return false;  // Invalid input
  }
}

void interface_Variable(CommandParameter& Parameters, bool set_status) {
  const char* location_cmd = Parameters.NextParameter();
  const char* variable_cmd = Parameters.NextParameter();

  int board_idx, channel_idx;
  if (isValidLocation(location_cmd, board_idx, channel_idx)) {
    // command sent for specific channel
    if (board_idx != -1 && channel_idx != -1) {
      // new variable
      if (strcmp(variable_cmd, "NTC_RT0") == 0) {
        if (set_status == true) {
          NTC_RT0[board_idx][channel_idx] = new uint16_t(Parameters.NextParameterAsInteger());
          Serial.print("NTC_RT0 updated to: ");
        } else if (set_status == false) {
          Serial.print("NTC_RT0 = ");
        }
        Serial.println(*NTC_RT0[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "NTC_Beta") == 0) {
        if (set_status == true) {
          NTC_Beta[board_idx][channel_idx] = new uint16_t(Parameters.NextParameterAsInteger());
          Serial.print("NTC_Beta updated to: ");
        } else if (set_status == false) {
          Serial.print("NTC_Beta = ");
        }
        Serial.println(*NTC_Beta[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_Setpoint") == 0) {
        if (set_status == true) {
          PID_Setpoint[board_idx][channel_idx] = new double(Parameters.NextParameterAsDouble());
          Serial.print("PID_Setpoint updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_Setpoint = ");
        }
        Serial.println(*PID_Setpoint[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_Output_Min") == 0) {
        if (set_status == true) {
          PID_Output_Min[board_idx][channel_idx] = new double(Parameters.NextParameterAsDouble());
          Serial.print("PID_Output_Min updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_Output_Min = ");
        }
        Serial.println(*PID_Output_Min[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_Output_Max") == 0) {
        if (set_status == true) {
          PID_Output_Max[board_idx][channel_idx] = new double(Parameters.NextParameterAsDouble());
          Serial.print("PID_Output_Max updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_Output_Max = ");
        }
        Serial.println(*PID_Output_Max[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_KP") == 0) {
        if (set_status == true) {
          PID_KP[board_idx][channel_idx] = new double(Parameters.NextParameterAsDouble());
          Serial.print("PID_KP updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_KP = ");
        }
        Serial.println(*PID_KP[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_KI") == 0) {
        if (set_status == true) {
          PID_KI[board_idx][channel_idx] = new double(Parameters.NextParameterAsDouble());
          Serial.print("PID_KI updated to: ");
        } else if (set_status == false) {
          Serial.print("PID_KI = ");
        }
        Serial.println(*PID_KI[board_idx][channel_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "PID_KD") == 0) {
        if (set_status == true) {
          PID_KD[board_idx][channel_idx] = new double(Parameters.NextParameterAsDouble());
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
          PWM_driver_address[board_idx] = new uint8_t(Parameters.NextParameterAsInteger());
          Serial.print("PWM_driver_address updated to: ");
        } else if (set_status == false) {
          Serial.print("PWM_driver_address = ");
        }
        Serial.println(*PWM_driver_address[board_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "NTC_CS_DIP") == 0) {
        if (set_status == true) {
          NTC_CS_DIP[board_idx] = new uint8_t(Parameters.NextParameterAsInteger());
          Serial.print("NTC_CS_DIP updated to: ");
        } else if (set_status == false) {
          Serial.print("NTC_CS_DIP = ");
        }
        Serial.println(*NTC_CS_DIP[board_idx]);
      }
      // new variable
      else if (strcmp(variable_cmd, "NTC_V_ref") == 0) {
        if (set_status == true) {
          NTC_V_ref[board_idx] = new double(Parameters.NextParameterAsDouble());
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
  interface_Variable(Parameters, false);
}

void Set_Variable(CommandParameter& Parameters) {
  interface_Variable(Parameters, true);
}

void Start_Channel(CommandParameter& Parameters) {
  const char* location_cmd = Parameters.NextParameter();
  const char* variable_cmd = Parameters.NextParameter();
  int board_idx, channel_idx;
  if (isValidLocation(location_cmd, board_idx, channel_idx)) {
    *Channel_output_flags[board_idx] = *Channel_output_flags[board_idx] | (1 << channel_idx);
    Serial.println("Channel started.");
  }
}

void Stop_Channel(CommandParameter& Parameters) {
  const char* location_cmd = Parameters.NextParameter();
  const char* variable_cmd = Parameters.NextParameter();
  int board_idx, channel_idx;
  if (isValidLocation(location_cmd, board_idx, channel_idx)) {
    *Channel_output_flags[board_idx] = *Channel_output_flags[board_idx] & ~(1 << channel_idx);
    Serial.println("Channel stopped.");
  }
}
