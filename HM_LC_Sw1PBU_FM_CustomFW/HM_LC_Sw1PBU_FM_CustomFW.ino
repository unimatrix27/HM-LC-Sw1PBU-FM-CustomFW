//- load library's --------------------------------------------------------------------------------------------------------
#include "HM_LC_Sw1PBU_FM_CustomFW.h"
#include "Register.h"																	// configuration sheet
#include <Buttons.h>																	// remote buttons library
#include <Relay.h>

#ifdef SER_DBG
//
//- serial communication --------------------------------------------------------------------------------------------------
const uint8_t helptext1[] PROGMEM = {													// help text for serial console
  "\r\n"
    "Available commands:" "\r\n"
    "  p                - start pairing with master" "\r\n"
    "  r                - reset device" "\r\n"
    "  b[0]  b[n]  s    - send a string, b[0] is length (50 bytes max)" "\r\n"
    "\r\n"
    "  c                - print configuration" "r\n"
    "\r\n"
    "  $nn for HEX input (e.g. $AB,$AC ); b[] = byte, i[]. = integer " "\r\n"
};
const InputParser::Commands cmdTab[] PROGMEM = {
  { 
    'p', 0, sendPairing     }
  ,
  { 
    'r', 0, resetDevice     }
  ,
  { 
    's', 1, sendCmdStr     }
  ,
  { 
    'b', 1, buttonSend     }
  ,
  { 
    'c', 0, printConfig     }
  ,
  { 
    0     }
};
InputParser parser (50, cmdTab);
#endif

//- homematic communication -----------------------------------------------------------------------------------------------
HM::s_jumptable jTbl[] = {																// jump table for HM communication
  // byte3, byte10, byte11, function to call											// 0xff means - any byte
  { 
    0x01, 0xff, 0x0e, HM_Status_Request     }
  ,
  { 
    0x11, 0x04, 0x00, HM_Reset_Cmd     }
  ,
  { 
    0x01, 0xff, 0x06, HM_Config_Changed     }
  ,
  { 
    0x00     }
};
Buttons button[3];																		// declare remote button object
Relay   relay[2];

//- main functions --------------------------------------------------------------------------------------------------------
void setup() {
#ifdef SER_DBG
  Serial.begin(57600);																// serial setup
  Serial << F("Starting sketch...\r\n");												// ...and some information
  Serial << pCharPGM(helptext1) << "\r\n";
  Serial << F("freeMem: ") << freeMem() << F(" byte") <<"\r\n";
#endif
  hm.cc.config(4,5,6,7,10,11);														// CS, MOSI, MISO, SCK, GDO0, Interrupt
  hm.statusLed.config(0, 0);															// configure the status led pin
  hm.statusLed.set(STATUSLED_1, STATUSLED_MODE_BLINKFAST, 3);
#ifdef SER_DBG
  Serial << "\npair: " << pHex(regs.ch0.l0.pairCentral,3) << "\r\n";
#endif	
  // setup battery measurement
  /*	hm.battery.config(
   		BATTERY_MODE_EXTERNAL_MESSUREMENT, 7, 1, BATTERY_MEASSUREMENT_FACTOR, 10000
   	);
   	hm.battery.setMinVoltage(BATTERY_MIN_VOLTAGE);*/

  hm.setPowerMode(0);																	// power mode for HM device
  hm.init();																			// initialize the hm module
    	button[0].regInHM(0,&hm);															// register buttons in HM per channel, handover HM class pointer
    	button[0].config(15, &buttonEvent);													// configure button on specific pin and handover a function pointer to the main sketch
    	button[1].regInHM(1,&hm);
    	button[1].config(14, &buttonEvent);
    	button[2].regInHM(2,&hm);
    	button[2].config(8, &buttonEvent);
        relay[0].regInHM(3,&hm);                                                                                                  // register relay class in HM to respective channel
        relay[0].config(&initRelay,&switchRelay,2,2);	                                                                          // init function, switch function, min delay, random delay for transmitting status message
        relay[1].regInHM(4,&hm);                                                                                                   
        relay[1].config(&initRelay,&switchRelay,2,2);


  byte rr = MCUSR;
  MCUSR =0;
}

void loop() {
#ifdef SER_DBG
  parser.poll();	
#endif																	// handle serial input from console
    	hm.poll();																			// poll the HM communication
}


//- HM functions ----------------------------------------------------------------------------------------------------------
void HM_Status_Request(uint8_t *data, uint8_t len) {
  Serial << F("status request, data: ") << pHex(data,len) << "\r\n";
}
void HM_Reset_Cmd(uint8_t *data, uint8_t len) {
  Serial << F("reset, data: ") << pHex(data,len) << "\r\n";
  hm.send_ACK();																		// send an ACK
  if (data[0] == 0) hm.reset();														// do a reset only if channel is 0
}
void HM_Config_Changed(uint8_t *data, uint8_t len) {
  Serial << F("config changed, data: ") << pHex(data,len) << "\r\n";
}
void buttonEvent(uint8_t idx, uint8_t state) {	
  Serial << F("bE, cnl: ") << idx << F(", state: ") << state << "\r\n";
}

void initRelay() {
 	digitalWrite(PIN_RELAY,0);
 	pinMode(PIN_RELAY,OUTPUT);
 }
 void switchRelay(uint8_t on) {
 	if (on) {
             digitalWrite(PIN_RELAY,1);
             hm.statusLed.set(STATUSLED_1,STATUSLED_MODE_ON,0);
 
 	} else {
 	     digitalWrite(PIN_RELAY,0);
             hm.statusLed.set(STATUSLED_1,STATUSLED_MODE_OFF,0);	
 	}
 }
 
#ifdef SER_DBG
//- config functions ------------------------------------------------------------------------------------------------------
void sendPairing() {																	// send the first pairing request
  hm.startPairing();
}
void resetDevice() {
  Serial << F("reset device, clear eeprom...\n");
  hm.reset();
  Serial << F("reset done\n");
}
void sendCmdStr() {																		// reads a sndStr from console and put it in the send queue
  memcpy(hm.send.data,parser.buffer,parser.count());									// take over the parsed byte data
  Serial << F("s: ") << pHexL(hm.send.data, hm.send.data[0]+1) << "\r\n";				// some debug string
  hm.send_out();																		// fire to send routine
}
void buttonSend() {
  uint8_t cnl, lpr;
  parser >> cnl >> lpr;

  Serial << "button press, cnl: " << cnl << ", looong press: " << lpr << "\r\n";			// some debug message
  hm.sendPeerREMOTE(cnl,lpr,0);														// parameter: button/channel, long press, battery
}
void printConfig() {
  hm.printConfig();
}





#endif


