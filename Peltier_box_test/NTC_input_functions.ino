uint16_t Read_MCP3304(uint8_t Channel, uint8_t Chip_select) {
  byte inByte = 0;                      // incoming byte from the SPI
  uint16_t result = 0;                  // result to return
  byte frame = 0b00011000 | (Channel);  //Single ended mode
  // Serial.print("frame to send: ");
  // Serial.println(frame, BIN);
  digitalWrite(Chip_select, LOW);
  SPI.transfer(frame);
  result = SPI.transfer16(0x0000);  //(Channel & 1) << 15);
  digitalWrite(Chip_select, HIGH);
  //Serial.print("Data recieved: ");
  //Serial.print(result,HEX);
  //Serial.print("   ");
  //Serial.println(result,BIN);
  result = result & 0b0001111111111111;
  return result;
}

void Read_NTC_inputs() {
    for (uint8_t board = 0; board < Num_NTC_boards; board++) {
      for (uint8_t channel = 0; channel < Inputs_per_board; channel++) {
        uint16_t temp = *NTC_Raw_Readings[board][channel][NTC_Raw_Readings_position];
        *NTC_Raw_Readings[board][channel][NTC_Raw_Readings_position] = Read_MCP3304(channel, NTC_CS_pins[*NTC_CS_DIP[board]]);
    *NTC_Raw_average[board][channel] +=((*NTC_Raw_Readings[board][channel][NTC_Raw_Readings_position]-temp)*1.0)/(Moving_average_window*1.0);
        *NTC_R_readings[board][channel] = ADC_to_R(*NTC_Raw_average[board][channel], *NTC_R1[board][channel]);
        *NTC_T_readings[board][channel] = R_to_Celsius(*NTC_R_readings[board][channel], *NTC_R_inf[board][channel], *NTC_Beta[board][channel]);
      }
    }
  // }
      NTC_Raw_Readings_position++;
    if (NTC_Raw_Readings_position>(Moving_average_window-1)){
      NTC_Raw_Readings_position=0;
    }
}

double ADC_to_R(uint16_t ADC_raw, uint16_t R_1) {
  double Resistance = R_1 / ((8191.0 / ADC_raw) - 1.0);
  return Resistance;
}

double R_to_Celsius(double R_T, double R_inf, uint16_t Beta) {
  double Temperature = (Beta / (log(R_T / R_inf))) - 273.15;
  return Temperature;
}