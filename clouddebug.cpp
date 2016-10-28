#include "Particle.h"

// In order to use this with full debugging, you need a debug build of system firmware
// as well as this app.

// Adds debugging output over Serial USB
// ALL_LEVEL, TRACE_LEVEL, DEBUG_LEVEL, INFO_LEVEL, WARN_LEVEL, ERROR_LEVEL, PANIC_LEVEL, NO_LOG_LEVEL
SerialDebugOutput debugOutput(9600, ALL_LEVEL);

// STARTUP(cellular_credentials_set("broadband", "", "", NULL));
// STARTUP(WiFi.selectAntenna(ANT_INTERNAL));

SYSTEM_MODE(SEMI_AUTOMATIC);


/* Function prototypes -------------------------------------------------------*/
int tinkerDigitalRead(String pin);
int tinkerDigitalWrite(String command);
int tinkerAnalogRead(String pin);
int tinkerAnalogWrite(String command);
const char *securityString(int value); // forward declaration
void wifi_scan_callback(WiFiAccessPoint* wap, void* data); // forward declaration

/* Constants -----------------------------------------------------------------*/
const unsigned long STARTUP_WAIT_TIME_MS = 5000;
const unsigned long CONNECT_WAIT_TIME_MS = 60000;

/* Global Variables-----------------------------------------------------------*/

enum State { STARTUP_STATE, WIFI_REPORT_STATE, CONNECT_WAIT_STATE, CONNECT_REPORT_STATE, CLOUD_CONNECT_WAIT_STATE, CLOUD_CONNECTED_STATE, DISCONNECT_STATE, IDLE_STATE };
State state = STARTUP_STATE;
unsigned long stateTime = 0;


/* This function is called once at start up ----------------------------------*/
void setup()
{
	Serial.begin(9600);

	//Setup the Tinker application here

	//Register all the Tinker functions
	Particle.function("digitalread", tinkerDigitalRead);
	Particle.function("digitalwrite", tinkerDigitalWrite);

	Particle.function("analogread", tinkerAnalogRead);
	Particle.function("analogwrite", tinkerAnalogWrite);
}

/* This function loops forever --------------------------------------------*/
void loop() {

	switch(state) {
	case STARTUP_STATE:
		if (millis() - stateTime >= STARTUP_WAIT_TIME_MS) {
			state = WIFI_REPORT_STATE;
		}
		break;

	case WIFI_REPORT_STATE:
		// Running in semi-automatic mode, turn on WiFi before beginning
		WiFi.on();

		// If WiFi has been configured, print out the configuration (does not include passwords)
		if (WiFi.hasCredentials()) {
			Serial.printlnf("configured credentials:");
			WiFiAccessPoint ap[5];
			int found = WiFi.getCredentials(ap, 5);
			for(int ii = 0; ii < found; ii++) {
				Serial.printlnf("ssid=%s security=%s cipher=%d", ap[ii].ssid, securityString(ap[ii].security), ap[ii].cipher);
			}
		}

		// Print out nearby access points
		Serial.printlnf("available access points:");
		WiFi.scan(wifi_scan_callback);

		// Connect to WiFi (but not cloud)
		Serial.println("connecting to WiFi");
		WiFi.connect();
		state = CONNECT_WAIT_STATE;
		stateTime = millis();
		break;

	case CONNECT_WAIT_STATE:
		if (WiFi.ready()) {
			state = CONNECT_REPORT_STATE;
		}
		break;

	case CONNECT_REPORT_STATE:
		{
			Serial.println("connected to WiFi!");

			Serial.printlnf("localIP=%s", WiFi.localIP().toString().c_str());
			Serial.printlnf("subnetMask=%s", WiFi.subnetMask().toString().c_str());

			IPAddress gatewayIP = WiFi.gatewayIP();
			Serial.printlnf("gatewayIP=%s", gatewayIP.toString().c_str());

			IPAddress dnsServerIP = WiFi.dnsServerIP();
			Serial.printlnf("dnsServerIP=%s (often 0.0.0.0)", dnsServerIP.toString().c_str());
			Serial.printlnf("dhcpServerIP=%s (often 0.0.0.0)", WiFi.dhcpServerIP().toString().c_str());

			byte bssid[6];
			WiFi.BSSID(bssid);
			// Serial.printlnf("BSSID=%02X:%02X:%02X:%02X:%02X:%02X", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);


			if (gatewayIP) {
				int count = WiFi.ping(gatewayIP, 1);
				Serial.printlnf("ping gateway=%d", count);
			}
			if (dnsServerIP) {
				int count = WiFi.ping(dnsServerIP, 1);
				Serial.printlnf("ping dnsServerIP=%d", count);
			}

			{
				IPAddress addr = IPAddress(8,8,8,8);
				int count = WiFi.ping(addr, 1);
				Serial.printlnf("ping addr %s=%d", addr.toString().c_str(), count);
			}

			{
				IPAddress addr;

				for(size_t tries = 1; tries <= 3; tries++) {
					addr = WiFi.resolve("device.spark.io");
					if (addr) {
						break;
					}
					Serial.printlnf("failed to get device.spark.io from DNS, try %d", tries);
					delay(2000);
				}
				Serial.printlnf("device.spark.io=%s", addr.toString().c_str());

				if (addr) {
					// This will always fail
					// int count = WiFi.ping(addr, 1);
					// Serial.printlnf("ping addr %s=%d", addr.toString().c_str(), count);

					TCPClient client;
					if (client.connect(addr, 5683)) {
						Serial.println("connected to device server CoAP (testing connection only)");
						client.stop();
					}
					else {
						Serial.println("could not connect to device server by CoAP");
					}
				}
			}

			Serial.println("connecting to cloud");
			Particle.connect();
		}

		state = CLOUD_CONNECT_WAIT_STATE;
		break;

	case CLOUD_CONNECT_WAIT_STATE:
		if (Particle.connected()) {
			Serial.println("connected to the cloud!");
			state = CLOUD_CONNECTED_STATE;
		}
		break;

	case IDLE_STATE:
	case CLOUD_CONNECTED_STATE:
		break;

	}

}


const char *securityString(int value) {
	const char *sec = "unknown";
	switch(value) {
	case WLAN_SEC_UNSEC:
		sec = "unsecured";
		break;

	case WLAN_SEC_WEP:
		sec = "wep";
		break;

	case WLAN_SEC_WPA:
		sec = "wpa";
		break;

	case WLAN_SEC_WPA2:
		sec = "wpa2";
		break;

	case WLAN_SEC_NOT_SET:
		sec = "not set";
		break;
	}
	return sec;
}

void wifi_scan_callback(WiFiAccessPoint* wap, void* data) {


	Serial.printlnf("SSID=%s security=%s channel=%d rssi=%d",
			wap->ssid, securityString(wap->security), wap->channel, wap->rssi);

}


/*******************************************************************************
 * Function Name  : tinkerDigitalRead
 * Description    : Reads the digital value of a given pin
 * Input          : Pin
 * Output         : None.
 * Return         : Value of the pin (0 or 1) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerDigitalRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber < 0 || pinNumber > 7) return -1;

	if(pin.startsWith("D"))
	{
		pinMode(pinNumber, INPUT_PULLDOWN);
		return digitalRead(pinNumber);
	}
	else if (pin.startsWith("A"))
	{
		pinMode(pinNumber+10, INPUT_PULLDOWN);
		return digitalRead(pinNumber+10);
	}
#if Wiring_Cellular
	else if (pin.startsWith("B"))
	{
		if (pinNumber > 5) return -3;
		pinMode(pinNumber+24, INPUT_PULLDOWN);
		return digitalRead(pinNumber+24);
	}
	else if (pin.startsWith("C"))
	{
		if (pinNumber > 5) return -4;
		pinMode(pinNumber+30, INPUT_PULLDOWN);
		return digitalRead(pinNumber+30);
	}
#endif
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerDigitalWrite
 * Description    : Sets the specified pin HIGH or LOW
 * Input          : Pin and value
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerDigitalWrite(String command)
{
	bool value = 0;
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber < 0 || pinNumber > 7) return -1;

	if(command.substring(3,7) == "HIGH") value = 1;
	else if(command.substring(3,6) == "LOW") value = 0;
	else return -2;

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, value);
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		digitalWrite(pinNumber+10, value);
		return 1;
	}
#if Wiring_Cellular
	else if(command.startsWith("B"))
	{
		if (pinNumber > 5) return -4;
		pinMode(pinNumber+24, OUTPUT);
		digitalWrite(pinNumber+24, value);
		return 1;
	}
	else if(command.startsWith("C"))
	{
		if (pinNumber > 5) return -5;
		pinMode(pinNumber+30, OUTPUT);
		digitalWrite(pinNumber+30, value);
		return 1;
	}
#endif
else return -3;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogRead
 * Description    : Reads the analog value of a pin
 * Input          : Pin
 * Output         : None.
 * Return         : Returns the analog value in INT type (0 to 4095)
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerAnalogRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber < 0 || pinNumber > 7) return -1;

	if(pin.startsWith("D"))
	{
		return -3;
	}
	else if (pin.startsWith("A"))
	{
		return analogRead(pinNumber+10);
	}
#if Wiring_Cellular
	else if (pin.startsWith("B"))
	{
		if (pinNumber < 2 || pinNumber > 5) return -3;
		return analogRead(pinNumber+24);
	}
#endif
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogWrite
 * Description    : Writes an analog value (PWM) to the specified pin
 * Input          : Pin and Value (0 to 255)
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerAnalogWrite(String command)
{
	String value = command.substring(3);

	if(command.substring(0,2) == "TX")
	{
		pinMode(TX, OUTPUT);
		analogWrite(TX, value.toInt());
		return 1;
	}
	else if(command.substring(0,2) == "RX")
	{
		pinMode(RX, OUTPUT);
		analogWrite(RX, value.toInt());
		return 1;
	}

	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits

	if (pinNumber < 0 || pinNumber > 7) return -1;

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		analogWrite(pinNumber, value.toInt());
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		analogWrite(pinNumber+10, value.toInt());
		return 1;
	}
	else if(command.substring(0,2) == "TX")
	{
		pinMode(TX, OUTPUT);
		analogWrite(TX, value.toInt());
		return 1;
	}
	else if(command.substring(0,2) == "RX")
	{
		pinMode(RX, OUTPUT);
		analogWrite(RX, value.toInt());
		return 1;
	}
#if Wiring_Cellular
	else if (command.startsWith("B"))
	{
		if (pinNumber > 3) return -3;
		pinMode(pinNumber+24, OUTPUT);
		analogWrite(pinNumber+24, value.toInt());
		return 1;
	}
	else if (command.startsWith("C"))
	{
		if (pinNumber < 4 || pinNumber > 5) return -4;
		pinMode(pinNumber+30, OUTPUT);
		analogWrite(pinNumber+30, value.toInt());
		return 1;
	}
#endif
else return -2;
}
