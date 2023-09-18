#include <AutoPID.h>
#include <SPI.h>
#include <Adafruit_PWMServoDriver.h>
#include "SdFat.h"
#include "sdios.h"
#include "FreeStack.h"
#include "RTClib.h"
#include <math.h>
#include "CommandHandler.h"  // Part of MegunoLink library
#include <Adafruit_PWMServoDriver.h>

//Driver variables

#define MAX_Num_Driver_boards 26              //Required to size the compile time pointers arrays. Limited by hardware (6 bit I2C address) and lazy coding (26 letters)
#define Drivers_per_board 8                   //Defined by hardware
uint8_t Num_Driver_boards = 1;                //Assume only 1 until told otherwise
uint16_t PID_Time_Step_mS = 1000;             //Time between PID calcs running
const double PID_Setpoint_default = 25.0;     //Default values to start at if not told otherwise by user or config file
const double PID_Output_Min_default = -4095;  //Max supported by hardware (PWM driver adafruit 815)
const double PID_Output_Max_default = 4095;   //Max supported by hardware (PWM driver adafruit 815)
//Pointer arrays initialised at compile time so that objects/variables can be created at runtime, allowing hardware re-configuration without re-flashing software
uint8_t *Channel_output_flags[MAX_Num_Driver_boards];  //Bitwise flags for if a channel is on/off
char *PID_Input_map[MAX_Num_Driver_boards][Drivers_per_board][2];
AutoPID *PID[MAX_Num_Driver_boards][Drivers_per_board];
double *PID_Input[MAX_Num_Driver_boards][Drivers_per_board];       //Will point to relevant member of NTC_T_readings (i.e. which input channel controls which output driver)
double *PID_Setpoint[MAX_Num_Driver_boards][Drivers_per_board];    //Target temperature in Celsius
double *PID_Output[MAX_Num_Driver_boards][Drivers_per_board];      //12 bit pwm value output, signed
double *PID_Output_Min[MAX_Num_Driver_boards][Drivers_per_board];  //Minimum value for driving H-bridges. Limit based on hardwares thermal capabilities/peltier V/I curve
double *PID_Output_Max[MAX_Num_Driver_boards][Drivers_per_board];  //Maximum value for driving H-bridges. Limit based on hardwares thermal capabilities/peltier V/I curve
double *PID_KP[MAX_Num_Driver_boards][Drivers_per_board];          //PID loop gains
double *PID_KI[MAX_Num_Driver_boards][Drivers_per_board];
double *PID_KD[MAX_Num_Driver_boards][Drivers_per_board];
bool *Driver_polarity[MAX_Num_Driver_boards][Drivers_per_board];  //For flipping drive direction in code instead of manually re-wiring. 0 = unflipped, 1 = flipped
Adafruit_PWMServoDriver *PWM_driver[MAX_Num_Driver_boards];
uint8_t *PWM_driver_address[MAX_Num_Driver_boards];

//Input variables
#define Max_Num_NTC_boards 16                                                                                  //Required to size the compile time pointers arrays. Limited by hardware (number of available CS pins)
#define Inputs_per_board 8                                                                                     //Defined by hardware
uint8_t Num_NTC_boards = 1;                                                                                    //Assume only 1 until told otherwise
const double NTC_T0 = 298.15;                                                                                  //Temperature at which thermistors have quoted resistance
const double NTC_V_ref_default = 3.3;                                                                          //Reference voltage produced on daughter board. Any error should be resolved by the fact that the ADC uses the same reference voltage, but included for rigor
const uint16_t NTC_RT0_default = 10000;                                                                        //Thermistor resistance at T0 default
const uint16_t NTC_Beta_default = 3435;                                                                        //Thermistor Beta value default
const uint16_t NTC_R1_default = 6800;                                                                          //Potential divider fixed resistor value
const uint8_t NTC_CS_pins[Max_Num_NTC_boards] = { 0, 1, 6, 7, 8, 9, 14, 15, 16, 17, 18, 19, 22, 26, 27, 28 };  //Defined by hardware, these are the GPIO pins accessed by each DIP switch position on the daughter boards
uint8_t *NTC_CS_DIP[Max_Num_NTC_boards];                                                                       //Pointers to values for GPIO pins used as NTC_in ADC CS
double *NTC_V_ref[Max_Num_NTC_boards];                                                                         //Calibration arrays, adjust for increased accuracy
uint16_t *NTC_RT0[Max_Num_NTC_boards][Inputs_per_board];                                                       //Reference voltage produced on daughter board. Any error should be resolved by the fact that the ADC uses the same reference voltage, but included for rigor
uint16_t *NTC_Beta[Max_Num_NTC_boards][Inputs_per_board];                                                      //Thermistor resistance at T0
uint16_t *NTC_R1[Max_Num_NTC_boards][Inputs_per_board];                                                        //Thermistor Beta value
double *NTC_R_inf[Max_Num_NTC_boards][Inputs_per_board];                                                       //Pre-calculated part of thermistor calculations
uint16_t *NTC_Raw_Readings[Max_Num_NTC_boards][Inputs_per_board];                                              //Raw ADC readings
double *NTC_R_readings[Max_Num_NTC_boards][Inputs_per_board];                                                  //ADC readings converted into measured resistance of thermistor
double *NTC_T_readings[Max_Num_NTC_boards][Inputs_per_board];                                                  //Temperature measurement of thermistors in Celsius


uint16_t NTC_reading_interval = 1000;    //Time in mS between NTC readings
uint32_t Previous_Read_Time_Millis = 0;  //Stored millis() values for triggering NTC readings
uint32_t Current_Time_Millis = 0;

//SD card variables
#define SD_SCK_PIN 2  //This SPI bus is used for SD card and NTC ADC's
#define SD_COPI_PIN 3
#define SD_CIPO_PIN 4
#define SD_CS_PIN 5
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#define SPI_CLOCK SD_SCK_MHZ(1)  //Needs to be slow because of NTC_in ADC's
SdFat32 SD;
RTC_PCF8523 rtc;
File32 logfile;

char Filename[27];  //Char array to determine filename //EDITME adjust to correct length

#define KEY_MAX_LENGTH 40    // change it if key is longer
#define VALUE_MAX_LENGTH 30  // change it if value is longer
// #define FILE_NAME "config.txt"  //Filename for settings file
char FILE_NAME[30] = "Rig_settings.txt";  //Filename for settings file //EDITME add error for filenames that are too long?

//System variables
CommandHandler<10, 50> SerialCommandHandler;  //number of commands, then buffer length of command strings

bool Core0_boot_flag = 0;  //Flag to tell Core1 that Core0 has successfully booted
bool Core1_boot_flag = 0;  //Flag to tell Core0 that Core1 has successfully booted
bool USB_flag = 0;         //Flag to indicate if USB communications are active
bool SD_boot_flag = 0;     //Flag to indicate SD successfully booted
bool RTC_flag = 0;         //Flag to indicate RTC successfully booted
bool file_ready_flag = 0;  //EDITME not currently used?
bool Verbose_output = 1;   //Flag for debugging messages

uint32_t Debugging_timestamps[100];
uint8_t Debugging_timestamp_position = 0;

//Functions with optional arguements need to be given a prototype before setup() because IDE is odd
uint8_t Load_settings_uint8(uint8_t Default_value, char *Name, uint8_t Board_number = 255, uint8_t Channel_number = 255);
uint16_t Load_settings_uint16(uint16_t Default_value, char Name[KEY_MAX_LENGTH - 10], uint8_t Board_number = 255, uint8_t Channel_number = 255);
double Load_settings_double(double Default_value, char Name[KEY_MAX_LENGTH - 10], uint8_t Board_number = 255, uint8_t Channel_number = 255);
double *Load_settings_string_pointer(double *Default_value, char Name[KEY_MAX_LENGTH - 10], uint8_t Board_number, uint8_t Channel_number);


void setup() {
  delay(1000);
  USB_flag = USB_setup(115200, 1000);  //Iniitalise the USB, and set flag if present



  SerialCommandHandler.SetDefaultHandler(Cmd_Unknown);

  SerialCommandHandler.AddCommand(F("Re-initialise"), Cmd_Reinitialise);
  SerialCommandHandler.AddCommand(F("Save"), Cmd_Save_settings);
  SerialCommandHandler.AddCommand(F("Set", Set_Variable));
  SerialCommandHandler.AddCommand(F("Get", Get_Variable)); // EDIT add function
  SerialCommandHandler.AddCommand(F("Start", Start_Channel));
  SerialCommandHandler.AddCommand(F("Stop", Stop_Channel));
  // SerialCommandHandler.AddCommand(F("NTC_CS="), Cmd_Set_NTC_CS); //EDITME add function

  Core0_boot_flag = 1;        //Flag to make Core1 wait for Core0
  while (!Core1_boot_flag) {  //Wait for Core1 to finish its setup
    delay(10);
  }
}

void setup1() {

  while (!Core0_boot_flag) {  //Wait for Core0 to finish its setup
    delay(10);
  }

  Initialise_SD(SD_SCK_PIN, SD_COPI_PIN, SD_CIPO_PIN, SD_CS_PIN);

  Wire.setSDA(20);  //Set pins for I2C
  Wire.setSCL(21);

  if (!rtc.begin()) {
    if (USB_flag) {
      Serial.println("Couldn't find RTC");
      Serial.flush();
    }
  } else {
    RTC_flag = 1;
  }

  //EDITME Write code to throw error and ask for manual reboot of Core1 if SD or RTC boot fails

  // DateTime boot_time = rtc.now();

  // while (!file_ready_flag) {  //Open a write file to test writing
  //   file_ready_flag = Create_logfile(boot_time, &Filename[0]);
  // }

  //EDITME add code to write file header
  // if (USB_flag) {
  //   Serial.println("Loading config file");
  // }

  //EDITME add code to open default file, check if alternative file should be loaded, and load that instead. 
  //EDITME add code to open file and append "last read" timestamp ending in comma 
  Num_NTC_boards = Load_settings_uint8(1, "Num_NTC_boards\0");        //Load number of NTC input boards from settings file, or assume just 1 until told otherwise // COMMENT: we should hard code this somehow?
  Num_Driver_boards = Load_settings_uint8(1, "Num_Driver_boards\0");  //Load number of output driver boards from settings file, or assume just 1 until told otherwise
  PID_Time_Step_mS = Load_settings_uint16(1000, "PID_Time_Step_mS\0");

  //Initialise input board settings, loading from SD file where available and using defaults otherwise
  for (uint8_t board = 0; board < Num_NTC_boards; board++) {
    NTC_CS_DIP[board] = new uint8_t(Load_settings_uint8(board, "NTC_CS\0", board));  //Assumes NTC input boards use DIP switch addresses of 0 upwards, no gaps and in order
    NTC_V_ref[board] = new double(Load_settings_double(NTC_V_ref_default, "NTC_V_ref\0", board));
    for (uint8_t channel = 0; channel < Inputs_per_board; channel++) {
      NTC_RT0[board][channel] = new uint16_t(Load_settings_uint16(NTC_RT0_default, "NTC_RT0\0", board, channel));
      NTC_Beta[board][channel] = new uint16_t(Load_settings_uint16(NTC_Beta_default, "NTC_Beta\0", board, channel));
      NTC_R1[board][channel] = new uint16_t(Load_settings_uint16(NTC_R1_default, "NTC_R1\0", board, channel));
      NTC_R_inf[board][channel] = new double(*NTC_RT0[board][channel] * exp((0.0 - *NTC_Beta[board][channel]) / NTC_T0));
      NTC_Raw_Readings[board][channel] = new uint16_t(0);
      NTC_R_readings[board][channel] = new double(0);
      NTC_T_readings[board][channel] = new double(0);
    }
  }

  //Initialise output board settings, loading from SD file where available and using defaults otherwise
  for (uint8_t board = 0; board < Num_Driver_boards; board++) {
    Channel_output_flags[board] = new uint8_t(Load_settings_uint8(0, "Channel_autostart_flags\0", board));  //Assume all ouputs are off unless told otherwise 
    PWM_driver_address[board] = new uint8_t(Load_settings_uint8(board, "PWM_driver_address\0", board));     //Assumes PWM drivers use solder address of 0 upwards, no gaps and in order. Addresses satart at 0x40 and increment upwards according to solder jumpers (offset handled by initiator code)
    PWM_driver[board] = new Adafruit_PWMServoDriver(*PWM_driver_address[board] + 0x40);
    PWM_driver[board]->begin();
    PWM_driver[board]->setPWMFreq(1600);  //Max allowable with hardware

    Adafruit_PWMServoDriver *PWM_driver[MAX_Num_Driver_boards];
    uint8_t *PWM_driver_address[MAX_Num_Driver_boards];
    for (uint8_t driver = 0; driver < Drivers_per_board; driver++) {
      PID_Input_map[MAX_Num_Driver_boards][Drivers_per_board][0] = new char;
      PID_Input_map[MAX_Num_Driver_boards][Drivers_per_board][1] = new char;
      PID_Input[board][driver] = Load_settings_string_pointer(NTC_T_readings[board][driver], "PID_Input\0", board, driver);  //Assumes outputs are controlled by corresponding inputs. A1 to A1, B3 to B3 etc
      PID_Setpoint[board][driver] = new double(Load_settings_double(NTC_V_ref_default, "PID_Setpoint\0", board, driver));
      PID_Output[board][driver] = new double();
      PID_Output_Min[board][driver] = new double(Load_settings_double(PID_Output_Min_default, "PID_Output_Min\0", board, driver));
      PID_Output_Max[board][driver] = new double(Load_settings_double(PID_Output_Max_default, "PID_Output_Max\0", board, driver));
      PID_KP[board][driver] = new double(Load_settings_double(0, "PID_KP\0", board, driver));  //Guessing gain values is shooting blind, jsut set to 0 until told otherwise and channel won't output anything
      PID_KI[board][driver] = new double(Load_settings_double(0, "PID_KI\0", board, driver));
      PID_KD[board][driver] = new double(Load_settings_double(0, "PID_KD\0", board, driver));
      PID[board][driver] = new AutoPID(PID_Input[board][driver], PID_Setpoint[board][driver], PID_Output[board][driver], *PID_Output_Min[board][driver], *PID_Output_Max[board][driver], *PID_KP[board][driver], *PID_KI[board][driver], *PID_KD[board][driver]);  //Initialise PID objects with appropriate arguments
    }
  }

  Core1_boot_flag = 1;
}


void loop() {
  while (!Core1_boot_flag) {}  //Check if Core1 has booted. Required in case of reboot over UART

  SerialCommandHandler.Process();
}

void loop1() {
  Read_NTC_inputs();


//Do PID calcs and push to outputs
  for (uint8_t board = 0; board < Num_Driver_boards; board++) {
    for (uint8_t driver = 0; driver < Drivers_per_board; driver++) {

      if ((*Channel_output_flags[board] & (1 << driver)) >> driver) {  //If channel output flag is set
        PID[board][driver]->run();                                     //Run PID calcs (also starts PID if stopped)
        if (*Driver_polarity[board][driver]) {
          *PID_Output[board][driver] *= -1;  //Flip output if polarity flag is set
        }
        if (*PID_Output[board][driver] >= 0) {                                        //If PID output is positive
          PWM_driver[board]->setPWM((2 * driver), 0, 4096);                           //Set directional output high
          PWM_driver[board]->setPWM((2 * driver) + 1, 0, *PID_Output[board][driver]);  //Ouput PID calc to PWM output
        } else {                                                                      //PID output is negative
          PWM_driver[board]->setPWM((2 * driver), 4096, 0);                           //Set directional output low
          PWM_driver[board]->setPWM((2 * driver) + 1, 0, *PID_Output[board][driver]);  //Ouput PID calc to PWM output
        }
      } else if (!PID[board][driver]->isStopped()) {  //If channel output flag is off, but PID is still running
        PID[board][driver]->stop();                   //Stop PID loop
      }
    }
  }

}
