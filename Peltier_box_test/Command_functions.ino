void Cmd_Unknown() {
  Serial.println(F("I don't know that command. Try another. "));
}

void Cmd_Reinitialise(CommandParameter &Parameters) {
  rp2040.restartCore1();
}

void Cmd_Save_settings(CommandParameter &Parameters) {
 const char *Filename_from_CMD=Parameters.NextParameter();
    // if (Verbose_output) {
    //   Serial.print("Filename: ");
    //   Serial.println(Filename_from_CMD);
    // }
  if (Save_settings_to_file(Filename_from_CMD)) {
    if (Verbose_output) {
      Serial.println("Settings saved");
    }
  }
}

void getChannel(const char* input, int& board_idx, int& channel_idx) {
  char boardChar = input[0];
  char channelChar = input[1];
  if (boardChar >= 'A' && boardChar <= 'Z' && channelChar >= '0' && channelChar <= '9') {
    board_idx = boardChar - 'A';
    channel_idx = channelChar - '0'; // this is for 1-8 channel names
  }
}

void getBoard(const char* input, int& board_idx, int& channel_idx) {
  char boardChar = input[0];
  if (boardChar >= 'A' && boardChar <= 'Z') {
    board_idx = boardChar - 'A';
  }
}

bool isValidLocation(const char* input, int& board_idx, int& channel_idx) {
  if (input == "all") {
    board_idx = -1
    channel_idx = -1
    return 1; // "all" is a valid input
  } else if (input.length() == 1 && isalpha(input[0])) {
    board_idx = input[0]
    channel_idx = -1
    return 1; // Single letter is valid
  } else if (input.length() == 2 && isalpha(input[0]) && isdigit(input[1])) {
    board_idx = input[0]
    channel_idx = input[1]
    return 1; // Letter-number pair is valid
  } else {
    return 0; // Invalid input
  }
}

bool isCorrectLocation(indices, board_idx, channel_idx){
  bool validity
  if (board_idx<0){
    if (indices==0){
      validity = 1
    }
    else {
      Serial.println(F("Invalid location for this variable: please specify a valid board."));
      validity = 0
    }
  }
  if (channel_idx<0){
    if (indices==0 || indices==1){
      validity = 1
    }
    else {
      Serial.println(F("Invalid location for this variable: please specify a valid channel."));
      validity = 0
    }
  }

  if (indices == 2 || indices){
    if (board_idx<0 {
      Serial.println(F("Invalid location: please specify a valid board."));
    }
    if (channel_idx<0){
      Serial.println(F("Invalid location: please specify a valid channel."));
    }
  if (indices == 1){
    if (board_idx<0 {
      Serial.println(F("Invalid location: please specify a valid board."));
    }
  }
  }
}

void Set_Variable(CommandParameter &Parameters) {
  char location_cmd = NextParameter();
  char variable_cmd = NextParameter();
  
  


  // list of all settings that can be changed - Review to only include settings we want
  // edit: add error messages when board and channel aren't correctly specified
  // edit: add confirmation step?
  // edit: add safety checks to settings?

  if (isValidLocation(location_cmd)){
      int board, channel;
      //getChannel(location_cmd, board, channel);
      switch (variable_cmd){
        case 'NTC_RT0':
          NTC_RT0[board][channel] = NextParameterAsInteger();
          break;
        case 'NTC_Beta':
          NTC_Beta[board][channel] = NextParameterAsInteger();
          break;
        case 'PID_Setpoint':
          PID_Setpoint[board][channel] = NextParameterAsDouble();
          break;
        case 'PID_Output_Min':
          PID_Output_Min[board][channel] = NextParameterAsDouble();
          break;
        case 'PID_Output_Max':
          PID_Output_Max[board][channel] = NextParameterAsDouble();
          break;
        case 'PID_KP':
          PID_KP[board][channel] = NextParameterAsDouble();
          break;
        case 'PID_KI':
          PID_KI[board][channel] = NextParameterAsDouble();
          break;
        case 'PID_KD':
          PID_KD[board][channel] = NextParameterAsDouble();
          break;
        default:
          Serial.println(F("Variable does not exist, must be applied to a specific board or is global."));
  }
  }
  else if (isValidLocation(location_cmd == 1)){
    int board;
    getBoard(location_cmd, board)
    switch (variable_cmd){
      case 'PWM_driver_address':
        PWM_driver_address[board] = NextParameterAs8bit(); // replace with function for importing 8 bit number
        break;
      case 'TC_CS_DIP':
        TC_CS_DIP[board] = NextParameterAs8bit()
        break;
      case 'NTC_V_ref':
        NTC_V_ref[board] = NextParameterAsDouble()
        break;
      default:
          Serial.println(F("Variable does not exist or is not board-specific."));
    }
  }
  else if (isValidLocation(location_cmd == 0)){
    switch (variable cmd){
      case 'PID_Time_Step_mS':
        PID_Time_Step_mS = NextParameterAsInteger();
        break;
      default:
        Serial.println(F("Variable does not exist or is not board-specific."));
    } 
  }
  else {
    Serial.println(F("Invalid location specified."));
  }
    // edit add default error message
  }
}

void Start_Channel(CommandParameter &Parameters)){
  char channel_cmd = NextParameter();
  int board, channel
  convertStringToIndices(channel_cmd, board, channel)
  Channel_output_flags[board] |= (1 << channel);
}
