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

void convertStringToIndices(const char* input, int& board_idx, int& channel_idx) {
  if (strlen(input) >= 2) {
    char boardChar = input[0];
    char channelChar = input[1];

    if (boardChar >= 'A' && boardChar <= 'Z' && channelChar >= '0' && channelChar <= '9') {
      board_idx = boardChar - 'A';
      channel_idx = channelChar - '0'; // this is for 1-8 channel names
    }
  }
}

void Set_Variable(CommandParameter &Parameters) {
  char channel_cmd = NextParameter();
  char variable_cmd = NextParameter();
  
  int board, channel
  convertStringToIndices(channel_cmd, board, channel)

  switch (variable_cmd){
    case 'NTC_RT0':
      NTC_RT0 = NextParameterAsInteger()

  }

}

      NTC_RT0[board][channel] = new uint16_t(Load_settings_uint16(NTC_RT0_default, "NTC_RT0\0", board, channel));
      NTC_Beta[board][channel] = new uint16_t(Load_settings_uint16(NTC_Beta_default, "NTC_Beta\0", board, channel));
      NTC_R1[board][channel] = new uint16_t(Load_settings_uint16(NTC_R1_default, "NTC_R1\0", board, channel));
      NTC_R_inf[board][channel] = new double(*NTC_RT0[board][channel] * exp((0.0 - *NTC_Beta[board][channel]) / NTC_T0));
      NTC_Raw_Readings[board][channel] = new uint16_t(0);
      NTC_R_readings[board][channel] = new double(0);
      NTC_T_readings[board][channel] = new double(0);