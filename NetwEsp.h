
/*
 * https://github.com/esp8266/Arduino/blob/master/doc/installing.rst
 * https://github.com/esp8266/Arduino#using-git-version
 *
 * https://www.digikey.com/en/maker/blogs/2018/how-to-program-arduino-with-eclipse
 *
 *  Photon (4.8), 2018-09 (4.9), 2018-12 (4.10), 2019-03 (4.11), 2019-06 (4.12)
 * 
 * 
 * 2022-01-09 refactor ESP_CLIENT_RESET_TIMER and 
 * 			  - retry connection after 8 seconds by D(5), iotServer is keeping alive by freshing ping(5) every 7 secods.
 *            - disable the reboot. test to see if nodes stay disconnected after wifi down. 
 *            - when Wifi down for a long while keep retrying every minute.
 * 			  refactor ESP_CLIENT_NEXTSEND_TIMER to unstress uploading after client connection	
 * 
 * 2020-07-14 refactor to connect / timers: more robuust
 * */

 
// TODO cleanup connectSOcket / connect Wifi.

// #define DEBUG

#ifndef NETWEsp_H
#define NETWEsp_H

#define ESP_VERSION 20000

#include "NetwBase.h"
#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#define ESP_TIMER_COUNT 5

#define ESP_START_TIMER 0
#define ESP_CLIENT_CONNECT_TIMER 1
#define ESP_CLIENT_RESET_TIMER 2
#define ESP_WIFI_CONNECT_LOG_TIMER 3
#define ESP_CLIENT_NEXTSEND_TIMER 4
 

#define ESP_CLIENT_RECONNECT_DELAY 3
#define ESP_CLIENT_ALIVE_INTERVAL 12

/* Delay paramter for connection. */
#define WIFI_DELAY        500
/* Max SSID octets. */
#define MAX_SSID_LEN      32
/* Wait this much until device gets IP. */
#define MAX_CONNECT_TIME  30000



 
class NetwEsp : public NetwBase
{
public:

    WiFiClient client;

    ESP8266WiFiMulti wifiMulti;
	// ESP8266HTTPUpdateServer espUpdater;
	// Updater *httpUpdater;
	// ESP8266WebServer *server;



 	unsigned long 	timers[ESP_TIMER_COUNT];

	int		mode = WIFI_STA;  	// WIFI_STA, WIFI_AP
	bool  	WebStarted = false;

	// scan
	int 	ssidFoundCount = 0;
	int		ssidRssi = 0;
	int 	ssidIdx = 0;
	char 	ssid[33];
	char 	ssidPw[33];
	byte 	bssid[6];
	int 	ssidChannel = 0;

	long 	ap16=0;
	long 	ap16Uploaded=0;

    int  	port=0;
	bool 	startWebOnBoot=false;
	bool 	startWifi=false;

    int 	connectCount = 0;
	int 	connectCountUploaded = -1;
	int     connectionLost = 0;
	int 	connectionLostUploaded = -1;
    int 	resetConnCount = 0;
	int 	resetConnCountUploaded = -1;

    bool  	connectioWasAlive = false;	

	char strings[6][33];
	char ssid1[33];
	char pwd1[33];
	char ssid2[33];
	char pwd2[33];
	char svc[33];
	char upQueue[33];

	char ssidAP[33];
	char pwdAP[33];

	long 	parentVersion;

	NetwEsp( )
	{
		port=9003;
		// currentWifi = 0;
		isParent = true;
		// webServerStarted = false;
		initTimers(ESP_TIMER_COUNT);
		WiFi.mode(WIFI_OFF);
	}
	virtual ~NetwEsp(){}  // suppress warning

    void httpGetOta(ESP8266WebServer *server);
    void httpGetWifiSettings(ESP8266WebServer *server);
    void extractWifiSettings(ESP8266WebServer *server);
	void setServer(ESP8266WebServer *server);
 
    void monitorWiFi(void);
 
	// void wifiConnect(void);
	// void checkClientConnection(void);
	bool clientConnect(void);
	void resetSocket(void);

	void startAP(void);
	void startSTA(void);
 
	bool wifiConnected()  {	return WiFi.status() == WL_CONNECTED ;}
	bool socketConnected(){	return client.connected();}

	void stopAll()        {	client.stopAll();}
	long rssi(){ return WiFi.RSSI(); }
	unsigned long ap(void);

	void localCmd(int cmd, long val);
	void loopEsp(void);
    void disconnect(void);

	int write( RxData *rxData);
    void trace(const char* id);

	void initTimers(int count);
	bool isTime( int id);
	bool isTimerActive( int id );
	bool isTimerInactive( int id );
	bool isReady( int id);
	// void nextTimer( int id){ nextTimerMillis(id, 60 );}	
	void nextTimer( int id ){ nextTimerMillis(id, 60000L );}	
	void nextTimer( int id, int periode){ nextTimerMillis(id, periode * 1000L );}	
	void nextTimerMillis( int id, unsigned long periode);
	void timerOff( int id );

private:

};

#endif    //NETWEsp_H

/*
 * typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;
 *
 *
 * */

/*
 *  WiFi.mode(WIFI_STA);  Station Mode
 *  WiFi.begin(ssid, password, channel, bssid, connect)
		Meaning of parameters is as follows:
		ssid � a character string containing the SSID of Access Point we would like to connect to, may have up to 32 characters
		password to the access point, a character string that should be minimum 8 characters long and not longer than 64 characters
		channel of AP, if we like to operate using specific channel, otherwise this parameter may be omitted
		bssid � mac address of AP, this parameter is also optional
		connect � a boolean parameter that if set to false, will instruct module just to save the other parameters
		 without actually establishing connection to the access point
	WiFi.config(local_ip, gateway, subnet, dns1, dns2)

	WiFi.mode(WIFI_AP);  Soft Access Point
	WiFi.softAP(ssid)

		WiFi.softAP(ssid, password, channel, hidden)
		The first parameter of this function is required, remaining three are optional.
		Meaning of all parameters is as follows: �

		ssid � character string containing network SSID (max. 63 characters)
		password � optional character string with a password. For WPA2-PSK network it should be at least 8 character long. If not specified, the access point will be open for anybody to connect.
		channel � optional parameter to set Wi-Fi channel, from 1 to 13. Default channel = 1.
		hidden � optional parameter, if set to true will hide SSID

		The function will return true or false depending on result of setting the soft-AP.

		softAPConfig
		Configure the soft access point�s network interface.
		softAPConfig (local_ip, gateway, subnet)
		All parameters are the type of IPAddress and defined as follows:
		local_ip � IP address of the soft access point
		gateway � gateway IP address
		subnet � subnet mask

		WiFi.softAPdisconnect();
		WiFi.disconnect();
		WiFi.mode(WIFI_STA);
		delay(100);

 *  WiFi.softAPdisconnect (true);
 * */
