#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#define BATTERY_MEASSUREMENT_FACTOR 0.164										// Faktor Spannungsteiler externe Batteriemessung
#define BATTERY_MIN_VOLTAGE         22											// Faktor Spannungsteiler externe Batteriemessung
#define PIN_CURRENT 31
#define PIN_RELAY 12



void HM_Reset_Cmd(uint8_t *data, uint8_t len);
void HM_Status_Request(uint8_t *data, uint8_t len);
void HM_Config_Changed(uint8_t *data, uint8_t len);
void buttonEvent(uint8_t idx, uint8_t state);
void sendPairing();
void resetDevice();
void sendCmdStr();
void buttonSend();
void printConfig();
