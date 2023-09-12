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

