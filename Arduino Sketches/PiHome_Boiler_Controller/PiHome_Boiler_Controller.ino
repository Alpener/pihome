//  _____    _   _    _                             
// |  __ \  (_) | |  | |                           
// | |__) |  _  | |__| |   ___    _ __ ___     ___ 
// |  ___/  | | |  __  |  / _ \  | |_  \_ \   / _ \ 
// | |      | | | |  | | | (_) | | | | | | | |  __/
// |_|      |_| |_|  |_|  \___/  |_| |_| |_|  \___|
//
//    S M A R T   H E A T I N G   C O N T R O L 
// *****************************************************************
// *              Boiler Controller Relay Sketch                   *
// *            Version 0.31 Build Date 09/01/2019                 *
// *            Last Modification Date 10/07/2019                  *
// *                                          Have Fun - PiHome.eu *
// *****************************************************************

// Enable debug prints to serial monitor
#define MY_DEBUG

//Set MY_SPLASH_SCREEN_DISABLED to disable MySensors splash screen. (This saves 120 bytes of flash)
#define MY_SPLASH_SCREEN_DISABLED

//Define Sketch Name 
#define SKETCH_NAME "Boiler Relay"
//Define Sketch Version 
#define SKETCH_VERSION "0.31"

// Enable and select radio type attached
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

//Define this to use the IRQ pin of the RF24 module (optional). 
//#define MY_RF24_IRQ_PIN 8

// * - RF24_PA_MIN = -18dBm
// * - RF24_PA_LOW = -12dBm
// * - RF24_PA_HIGH = -6dBm
// * - RF24_PA_MAX = 0dBm
// Set LOW transmit power level as default, if you have an amplified NRF-module and
// power your radio separately with a good regulator you can turn up PA level.
// RF24_PA_MIN RF24_PA_LOW RF24_PA_HIGH RF24_PA_MAX RF24_PA_ERROR
#define MY_RF24_PA_LEVEL RF24_PA_MIN
//#define MY_DEBUG_VERBOSE_RF24

/**
 * @brief RF channel for the sensor net, 0-125.
 * Frequencies: 2400 Mhz - 2525 Mhz
 * @see https://www.nordicsemi.com/eng/nordic/download_resource/8765/2/42877161/2726
 * - 0 => 2400 Mhz (RF24 channel 1)
 * - 1 => 2401 Mhz (RF24 channel 2)
 * - 76 => 2476 Mhz (RF24 channel 77)
 * - 83 => 2483 Mhz (RF24 channel 84)
 * - 124 => 2524 Mhz (RF24 channel 125)
 * - 125 => 2525 Mhz (RF24 channel 126)
 * In some countries there might be limitations, in Germany for example only the range
 * 2400,0 - 2483,5 Mhz is allowed.
 * @see http://www.bundesnetzagentur.de/SharedDocs/Downloads/DE/Sachgebiete/Telekommunikation/Unternehmen_Institutionen/Frequenzen/Allgemeinzuteilungen/2013_10_WLAN_2,4GHz_pdf.pdf
 */
 
//Default RF channel Default is 76
#define MY_RF24_CHANNEL	91

//PiHome Boiler Controller Node ID 
#define MY_NODE_ID 100

//RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
#define MY_RF24_DATARATE RF24_250KBPS

//Enable Signing <Make Sure you Change Password>
//#define MY_SIGNING_SIMPLE_PASSWD "pihome"

//Enable Encryption This uses less memory, and hides the actual data. <Make Sure you Change Password>
//#define MY_ENCRYPTION_SIMPLE_PASSWD "pihome"

// Enable repeater functionality for this node
//#define MY_REPEATER_FEATURE

// Set baud rate to same as optibot
//#define MY_BAUD_RATE 9600

//set how long to wait for transport ready in milliseconds
//#define MY_TRANSPORT_WAIT_READY_MS 3000

//If Following LED Blink does not work then modify C:\Program Files (x86)\Arduino\libraries\MySensors_x_x_x\MyConfig.h 
#define MY_DEFAULT_ERR_LED_PIN 8
#define MY_DEFAULT_TX_LED_PIN 6
#define MY_DEFAULT_RX_LED_PIN 7
#define MY_WITH_LEDS_BLINKING_INVERSE

#define MY_DEFAULT_LED_BLINK_PERIOD 600

#include <MySensors.h>

#define RELAY_1  3  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay

int oldStatus = RELAY_OFF;
int COMMS = 0;
unsigned long WAIT_TIME = 300000; // Wait time (in milliseconds) best to keep it for 5 Minuts

void before()
{
	for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
		// Then set relay pins in output mode
		pinMode(pin, OUTPUT);
		// Set relay to last known state (using eeprom storage)
		//digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
	}
}

void setup(){

}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	//sendSketchInfo("Boiler Relay", "0.31");
	sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);

	for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
		// Register all sensors to gw (they will be created as child devices)
		present(sensor, S_BINARY);
	}
}

void loop(){
	//Safety function for Boiler: If Boiler Controller loses connection with gateway or RPI crashes boiler will turn off at set time. 
	wait(WAIT_TIME);
	if (COMMS == 1) {
		Serial.print("Comms Received Witin Time: \n");
		COMMS = 0;
	}else {
		Serial.print("NO Comms Received Witin Time!!! \n");
		Serial.print("Shutting Down Boiler \n");
		// Change relay state to Off
		digitalWrite(RELAY_1, RELAY_OFF);
	}
}

void sendHeartbeat()
{
}


void receive(const MyMessage &message)
{
	// We only expect one type of message from controller. But we better check anyway.
	if (message.type==V_STATUS) {
		
		//Set the Comms variable to 1 when v_status received 
		COMMS = 1;
		
		//digitalWrite(message.sensor-1+RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
		int RELAY_status = (message.sensor-1+RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
		
		// Write some debug info
		#ifdef MY_DEBUG
			Serial.print("New Status Received: ");
			Serial.println(RELAY_status);
			Serial.print("Old Status: ");
			Serial.println(oldStatus);
			Serial.print(" \n");
		#endif
		if (oldStatus == RELAY_ON && RELAY_status == RELAY_ON){
			//Change relay state to On 
			digitalWrite(message.sensor-1+RELAY_1, RELAY_ON);
			oldStatus = RELAY_status;
		}else {
			// Change relay state to Off
			digitalWrite(message.sensor-1+RELAY_1, RELAY_OFF);
			oldStatus = RELAY_status;
		}
	}
}
