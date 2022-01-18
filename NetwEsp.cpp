#include "Arduino.h"
#include "NetwEsp.h"

// #define DEBUG
 
/*



*/



PROGMEM const char* otaForm =
	"<html lang='en'><head><meta charset='UTF-8'>"
	"<meta name='viewport' content='width=device-width, initial-scale=1.5, maximum-scale=1.5, minimum-scale=1.5'>"
	"<title>J&amp;L IoT</title>"
	"<style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }"
	".butt { width: 50%;}</style>"

	"<script>"
	"function post(port, state) {"
	"var wijzig=document.getElementById('wijzig').checked;"
	"document.getElementById('poort').value = port;"
	"document.getElementById('staat').value = state;"
	"if(wijzig)"
	"{window.location.replace('details?port='+port);}"
	"else"
	"{document.getElementById('myForm').submit(); }}"

	"function getDetail(port) {"
	"window.location.replace('/details?port='+port);}"

	"function iotRefresh() {"
	"  var iotPath=window.location.pathname;"
	"  if( iotPath === '/set'){"
	"    setTimeout(function() {"
	"      window.location.replace('/');"
	"    }, 3000);"
	"} }"

	"function gotoParms() {"
	"  window.location.replace('/');"
	"}"


	"</script>"

	"</head><body>"

	"<form method='POST' action='/update' enctype='multipart/form-data'>"
	"<span onclick=\"if(event.ctrlKey)window.location.href='/settings'; else window.location.replace('/');\" >Uploading Over The Air (OTA)</span><br/><br/>"

	"<input type='file' name='update'>"
	"<input type='submit' value='Update'></form>" 

	"</body></html>"
	;


PROGMEM const char* wifiSettings =
	"<html lang='en'><head><meta charset='UTF-8'>"
	"<meta name='viewport' content='width=device-width, initial-scale=1.5, maximum-scale=1.5, minimum-scale=1.5'>"
	"<title>J&amp;L IoT</title>"
	"<style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }"
	".butt { width: 50%;}</style>"

	"<script>"
	"function post(port, state) {"
	"var wijzig=document.getElementById('wijzig').checked;"
	"document.getElementById('poort').value = port;"
	"document.getElementById('staat').value = state;"
	"if(wijzig)"
	"{window.location.replace('details?port='+port);}"
	"else"
	"{document.getElementById('myForm').submit(); }}"

	"function getDetail(port) {"
	"window.location.replace('/details?port='+port);}"

	"function iotRefresh() {"
	"  var iotPath=window.location.pathname;"
	"  if( iotPath === '/set'){"
	"    setTimeout(function() {"
	"      window.location.replace('/');"
	"    }, 3000);"
	"} }"

	"function gotoOta() {"
	"  window.location.replace('/up');"
	"}"
 
	"</script>"
	"</head><body>"
	"<form method='POST' action='/wifiParms' enctype='multipart/form-data'>"
	"<span onclick=\"if(event.ctrlKey)window.location.href='/up'; else window.location.replace('/');\" >Wifi Settings</span><br/><br/>"
	;



void NetwEsp::httpGetOta(ESP8266WebServer *server)
{
    server->sendHeader("Connection", "close");
    server->sendHeader("Access-Control-Allow-Origin", "*");	
	server->send ( 200, "text/html", otaForm );
}

void NetwEsp::httpGetWifiSettings(ESP8266WebServer *server)
{
	String formfields =
	"Main version: "+String(parentVersion)+" esp: "+String( ESP_VERSION)+"<br/>"
	"ssid1<input type='text' name='ssid' value='"+String(ssid1)+"'><br/>"
	"phrase1<input type='password' name='phrase' value='password'><br/>"
	"ssid2<input type='text' name='ssid2' value='"+String(ssid2)+"'><br/>"
	"phrase2<input type='password' name='phrase2' value='password'><br/>"
	"nodeid<input type='text' name='nodeid' value='"+String(nodeId)+"'><br/>"
	"connector<input type='text' name='connector' value='"+String(svc)+"'><br/>"
	"port<input type='text' name='port' value='"+String(port)+"'><br/>"
	"start<input type='text' name='start' value='"+String(upQueue)+"'><br/>"
	"web<input type='text' name='web' value='0'><br/>"
	"wifi<input type='text' name='wifi' value='0'><br/>"
	"<input type='submit' value='Update'></form>";
	;

	String html = wifiSettings+String(formfields)+"</body></html>";
	server->send ( 200, "text/html", html );
}


void NetwEsp::monitorWiFi()
{

  if (wifiMulti.run() != WL_CONNECTED)
  {
    if (connectioWasAlive == true)
    {
      connectioWasAlive = false;
	  connectionLost++;
      Serial.print("lost WiFi");
    }

	if(isTime(ESP_WIFI_CONNECT_LOG_TIMER)){
		nextTimerMillis(ESP_WIFI_CONNECT_LOG_TIMER, 500);
    	Serial.print(".");
	}
 
  } else if (connectioWasAlive == false) {

    connectioWasAlive = true;
    Serial.printf("@%ld wifi %s ", millis()/1000, WiFi.SSID().c_str());
	for(int j=0; j<6; j++){
		if(j>0) Serial.print(".");
		Serial.print( WiFi.BSSID()[j], HEX );	
	}
	Serial.println(); 
  }
}


// https://tttapa.github.io/ESP8266/Chap07%20-%20Wi-Fi%20Connections.html
// https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html


void  NetwEsp::startAP(void)  //TODO
{
	if(  WiFi.status() == WL_CONNECTED ){
		WiFi.disconnect();
		delay(500);
	}
 
	// he WiFi mode, as one of the wifi.STATION, wifi.SOFTAP, wifi.STATIONAP or wifi.NULLMODE
	if( WiFi.getMode() == WIFI_AP ){

		timerOff(ESP_START_TIMER);
		return;
	} 

	#ifdef DEBUG
		Serial.print("@"); Serial.print(millis()/1000) ; Serial.println(" Start startAP");
	#endif		


	WiFi.enableAP(true);
	WiFi.mode(WIFI_AP);

	bool suczeven =  WiFi.softAP(ssidAP, pwdAP, 10);	
	if(suczeven){

		// APStarted = true;

		#ifdef DEBUG
			Serial.print("@ "); Serial.print(millis()/1000); Serial.print(": startAP ok: ");  Serial.println (ssidAP );
		#endif
		timerOff(ESP_START_TIMER);

	} else {

		#ifdef DEBUG
			Serial.print("@ "); Serial.print(millis()/1000); Serial.print(": startAP failed: ");  Serial.println (ssidAP );
		#endif
	}
	 
}

void  NetwEsp::startSTA(void)   
{	
	// nextTimerMillis(ESP_WIFI_CONNECT_LOG_TIMER, 500);

	WiFi.setSleepMode(WIFI_NONE_SLEEP);
	WiFi.enableAP(false);
	WiFi.mode(WIFI_STA);

	if(WiFi.status() != WL_CONNECTED){

		#ifdef DEBUG
			Serial.print("@"); Serial.print(millis()/1000) ; Serial.println(" Start wifiMulti");
		#endif		

		// WiFi.setSleepMode(WIFI_NONE_SLEEP);
		wifiMulti.addAP(ssid1, pwd1);
		wifiMulti.addAP(ssid2, pwd2);
		// wifiMulti.run(15000);
		wifiMulti.run();
		ap16 = 0;
	}

	timerOff(ESP_START_TIMER);		
}

void NetwEsp::localCmd(int cmd, long val)
{
	uint8_t* BSSID;

	switch ( cmd)
	{
	case 1:
		WiFi.reconnect();
        break;
	case 2:
		WiFi.persistent(val>0);
        break;
	case 3:
		WiFi.mode(WIFI_STA);
  		WiFi.disconnect();
		WiFi.persistent(false);
		WiFi.scanDelete();
 		delay(500);
		Serial.print("Scan start ... ");   
		ssidRssi = -1000;
		ssidChannel = 0;
		ssidFoundCount = WiFi.scanNetworks();
		Serial.print(ssidFoundCount);
		Serial.println(" network(s) found");
		for (int i = 0; i < ssidFoundCount; i++)
		{
			BSSID = WiFi.BSSID(i);
			Serial.println();
   			Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
			for(int j=0; j<6; j++){
				Serial.print( BSSID[j], HEX );	
				Serial.print(".");	
			}
			Serial.println();

			if(WiFi.RSSI(i) > ssidRssi ){
 
				if( strncmp ( WiFi.SSID(i).c_str(), ssid1, 32  ) == 0
				){
					ssidIdx = i;
					strncpy(ssid, ssid1, 32);
					strncpy(ssidPw, pwd1, 32);
					ssidRssi = WiFi.RSSI(i);
					ssidChannel = WiFi.channel(i);
					memcpy(bssid, WiFi.BSSID(i), 6);
				}
				if( strncmp ( WiFi.SSID(i).c_str(), ssid2, 32  ) == 0
				){
					ssidIdx = i;
					strncpy(ssid, ssid2, 32);
					strncpy(ssidPw, pwd2, 32);					
					ssidRssi = WiFi.RSSI(i);
					ssidChannel = WiFi.channel(i);
					memcpy(bssid, WiFi.BSSID(i), 6);
				}
			}
		}
		// Serial.print("winnar:" );Serial.print( ssidIdx); Serial.print(" ");Serial.print(WiFi.SSID(ssidIdx));Serial.print(" ");Serial.println(WiFi.channel(ssidIdx));
		Serial.print("winnar:" ); 
		Serial.print(ssid);Serial.print(" ");Serial.print(ssidChannel); 
		Serial.print(" ");Serial.print(ssidPw);Serial.print(" ");
		for(int j=0; j<6; j++){
			Serial.print( bssid[j], HEX );	
			Serial.print(".");	
		}
		Serial.println(); 
 
		// myString.equals(myString2)	
        break;

	case 4:
		if(val == 1) mode = WIFI_AP;	;
		if(val == 2) mode = WIFI_STA;	;
        break;

	case 16:
		uploadFunc(16, ap16, millis() ); 		
        break;

	default:
		NetwBase::localCmd(cmd-100, val);
		break;
	}
}

void NetwEsp::disconnect(){

	WiFi.disconnect();

}
 
void NetwEsp::loopEsp()  // TODO  server or client ???? call only when in parent mode
{
	if( isTime(ESP_START_TIMER)
	 || WiFi.getMode() != mode
	){
		nextTimer(ESP_START_TIMER, 7);	
		if(mode == WIFI_STA) startSTA(); 
		if(mode == WIFI_AP) startAP();
		nextTimer(ESP_CLIENT_CONNECT_TIMER, 1);		
	}


	// When unable to connect to the Wifi then switch to AccessPoint
	if( mode == WIFI_STA
	 && WiFi.status() != WL_CONNECTED
	){
		if( millis() > 21000 
	 	 && millis() < 49000 	 
		){
			mode = WIFI_AP;
			return;
		} 
		wifiMulti.run();
	}

	// wait for Start finished
	if( isTimerActive(ESP_START_TIMER)
	 || mode == WIFI_AP
	){
		return;
	}  
 
	// wifiMulti.run();
	monitorWiFi();

	if(  WiFi.status() != WL_CONNECTED
	 || port <= 0
	){
		return;  // no client connection needed
	} 

	if(ap16==0){
		ap16 = ap();
	} 


	if( client.connected()
	){	
		nextTimer(ESP_CLIENT_CONNECT_TIMER, 3 );  //prevent stress

		if( client.available() > 0){
		
			#ifdef DEBUG
				 Serial.print("@"); Serial.print(millis()/1000) ; Serial.print(" available:");Serial.println(client.available() );
			#endif

			while (client.available())
			{
				pushChar(client.read());
			}

			resetConnCount = 0;
			nextTimer(ESP_CLIENT_RESET_TIMER, 8);
		} 
		
		if( isTime(ESP_CLIENT_RESET_TIMER) ){			
			resetSocket();
			return;
		} 

		findPayLoadRequest();  // check for input

		if(isReady(ESP_CLIENT_NEXTSEND_TIMER)
		){
			// nextTimerMillis(ESP_CLIENT_NEXTSEND_TIMER, 100);
			// nextTimerMillis(ESP_CLIENT_NEXTSEND_TIMER, 3);
			NetwBase::loop();  // write from buffer
		}


		if( uploadFunc!=0 
		 && isReady(ESP_CLIENT_NEXTSEND_TIMER)
		){			
			if(  connectCountUploaded != connectCount
			 && connectCount != 1
			){
				uploadFunc(7, connectCount, millis() ); 
				connectCountUploaded = connectCount;

			} else 	if(  resetConnCountUploaded != resetConnCount
			         &&  resetConnCount != 1
			){
				resetConnCountUploaded = resetConnCount;
				uploadFunc(4, resetConnCount -1, millis() ); 
				
			} else if(ap16 != ap16Uploaded ){
				// uploadFunc(16, ap16, millis() ); 
				// ap16Uploaded = ap16;
			}

		}

		if(errorFunc!=0 ){

			if( connectionLostUploaded != connectionLost
			){
				connectionLostUploaded = connectionLost;
				// errorFunc(7, 1000 + connectCountUploaded );
			} 
		}

	}

	if( ! client.connected()) clientConnect();
}
 

bool NetwEsp::clientConnect()
{
	if(isTimerActive(ESP_CLIENT_CONNECT_TIMER)
	 && ! isTime(ESP_CLIENT_CONNECT_TIMER)
	){
		return false;
	}

 
	connectCount++;
 
	nextTimer(ESP_CLIENT_CONNECT_TIMER, 3 );   // prevent stress 

	nextTimer(ESP_CLIENT_RESET_TIMER, ESP_CLIENT_ALIVE_INTERVAL);

	if (client.connect(svc, port) ){

		#ifdef DEBUG
			Serial.print("socket: "); Serial.print(svc);Serial.print(":");Serial.println(port);
		#endif		

		client.println(upQueue);	// subscribe for iotOut/7

		// nextTimerMillis(ESP_CLIENT_NEXTSEND_TIMER,250);
		nextTimerMillis(ESP_CLIENT_NEXTSEND_TIMER,100);


		return true;

	} else {

		#ifdef DEBUG
			Serial.print("@"); Serial.print(millis()/1000) ; Serial.println(" socket connect failed");
		#endif

		nextTimer(ESP_CLIENT_CONNECT_TIMER , resetConnCount>3 ? ESP_CLIENT_RECONNECT_DELAY * 3 : resetConnCount * ESP_CLIENT_RECONNECT_DELAY);
		return false;
	}
}


void NetwEsp::resetSocket()
{		
	resetConnCount++;
	nextTimer(ESP_CLIENT_RESET_TIMER, resetConnCount * 3);	


	bool rebootAllowed = true;
	if(checkForReboot!=0){
		rebootAllowed = checkForReboot();
	}

	if( resetConnCount < 3 
	){
		#ifdef DEBUG
			Serial.print("@"); Serial.print(millis()/1000) ; Serial.println("-------Send D");
		#endif

		txCmd('D', nodeId, 5 );
		// nextTimer(ESP_CLIENT_RESET_TIMER, 5);	
		if (resetConnCount == 1) nextTimer(ESP_CLIENT_RESET_TIMER, 3);   
		if (resetConnCount == 2) nextTimer(ESP_CLIENT_RESET_TIMER, 5);   
 

	// } else if( resetConnCount < 6 ||
	} else if(  
			 ! rebootAllowed
	){

		#ifdef DEBUG
			Serial.print("@"); Serial.print(millis()/1000) ; Serial.println("------client.stopAll");
		#endif

		client.stopAll();
		nextTimer(ESP_CLIENT_CONNECT_TIMER, 1);
		nextTimer(ESP_CLIENT_RESET_TIMER, 60); 
		if (resetConnCount == 3) nextTimer(ESP_CLIENT_RESET_TIMER, 7);   
		if (resetConnCount == 4) nextTimer(ESP_CLIENT_RESET_TIMER, 49);   
		// if (resetConnCount == 5) nextTimer(ESP_CLIENT_RESET_TIMER, 120);   
		// if (resetConnCount == 6) nextTimer(ESP_CLIENT_RESET_TIMER, 300);   

	// } else {

	// 	#ifdef DEBUG
	// 		Serial.print("@"); Serial.print(millis()/1000) ; Serial.println("------------rstConn reboot");
	// 	#endif
	// 	ESP.wdtDisable(); 
	// 	ESP.restart();   //software boot
	// 	while(true){delay(1);} ;
	}	 
}


void NetwEsp::extractWifiSettings(ESP8266WebServer *server)
{

	if (server->args() > 0 )
	{
		for ( uint8_t i = 0; i < server->args(); i++ )
		{

			// Serial.print("argName:"); Serial.println(server->argName(i));
			if(server->argName(i).equals("ssid"))
			{
				strncpy( ssid1, server->arg(i).c_str(), 32 );
			}

			if(server->argName(i).equals("phrase"))
			{
				if(server->arg(i) != String("password") )
				{
					strncpy( pwd1, server->arg(i).c_str(), 32 );
				}
			}

			if(server->argName(i).equals("ssid2"))
			{
				strncpy( ssid2, server->arg(i).c_str(), 32 );
			}

			if(server->argName(i).equals("phrase2"))
			{
				if(server->arg(i) != String("password") )
				{
					strncpy( pwd2, server->arg(i).c_str(), 32 );
				}
			}

			if(server->argName(i).equals("connector"))
			{
				strncpy( svc, server->arg(i).c_str(), 32 );
			}

			if(server->argName(i).equals("start"))
			{
				strncpy( upQueue, server->arg(i).c_str(), 32 );
			}

			if(server->argName(i).equals("nodeid"))
			{
				if(server->arg(i) != String(nodeId) )
				{
					nodeId = String(server->arg(i)).toInt();
				}
			}

			if(server->argName(i).equals("port"))
			{
				port = String(server->arg(i)).toInt();
			}

			if(server->argName(i).equals("web"))
			{
				startWebOnBoot = String(server->arg(i)).toInt()>0;
			}

			if(server->argName(i).equals("wifi"))
			{
				startWifi = String(server->arg(i)).toInt()>0;
			}
			

		}
	}
}

unsigned long NetwEsp::ap( void )  
{
int ssid_len;
// unsigned long lngSSID;

	ssid_len =  WiFi.SSID().length();

	// uint8 lastBssid = WiFi.BSSID()[5];
	return WiFi.SSID()[ssid_len-1] 
		| (WiFi.SSID()[ssid_len-2] << 8) 
		// | (WiFi.SSID()[ssid_len-3] << 16) 
		| (WiFi.channel() << 16) 
		| (WiFi.BSSID()[5] << 24);
}


int NetwEsp::write( RxData *rxData ) // opt: 0=cmd, 1=val, 2=all
{
 
	if(nodeId==0)
	{
		lastError =	ERR_TCP_SEND;
		return ERR_TCP_SEND;	
	}

	rxData->msg.conn = nodeId;
	serialize( &rxData->msg, strTmp);

	#ifdef DEBUG
		Serial.print(F("@"));Serial.print(millis()/1000 );Serial.print(F(" write(")); Serial.print(port); Serial.print(F(") payld="));   Serial.println(strTmp);Serial.flush();
	#endif

	if(! client.connected() )
	{
		bool succes = clientConnect();
		if(!succes){
 
			lastError =	ERR_TCP_SEND;
			return ERR_TCP_SEND;
		}
	}

	client.println (strTmp  );//U,23,40,11,2160,1560113198

	// sendCount++;

	// netwTimer = millis() + NETW_SEND_SERIAL_DATA_INTERVAL;
	
	// nextTimerMillis(NW_TIMEOUT_OVERALL, TWI_TIMEOUT);

	// if(nodeId==rxData->msg.node){

	// 	// pingTimer = millis() + PING_TIMER;
	// 	nextTimerMillis(NW_TIMER_PING, PING_TIMER);
	// } 

	return 0;
}


void NetwEsp::initTimers(int count)
{
	for(int i=0; i<count; i++){
		timers[i]=0L;
	}

	// if(! forceAP){
		nextTimer(ESP_START_TIMER, 0);
	// 	nextTimerMillis(ESP_WIFI_CONNECT_LOG_TIMER, 500);
	// }
}

// bool NetwEsp::isTime( int id){

// 	if(timers[id] == 0L){ return false; }

// 	unsigned long noww = millis();
// 	if(noww > timers[id]) return true;
// 	return (timers[id] - noww ) >  0x0fffffff;
// 	// long delta = millis() > timers[id]? millis()-timers[id] : timers[id]-millis();
// 	// return timers[id] != 0L && millis() >= timers[id] && delta < 0x0fffffff  ;
// }
bool NetwEsp::isTime( int id){
	if(timers[id] == 0L) return false;

	unsigned long delta = millis() > timers[id] ? millis() - timers[id] : timers[id] - millis() ;
	return delta > 0x0fffffff ? false : millis() >= timers[id];
	// if( millis() > timers[id]) return true;
	// return (timers[id] -  millis() ) >  0x0fffffff;  
}
bool NetwEsp::isReady( int id){
	if(timers[id] == 0L) return true;

	unsigned long delta = millis() > timers[id] ? millis() - timers[id] : timers[id] - millis() ;
	return delta > 0x0fffffff ? false : millis() >= timers[id]; 
}

bool NetwEsp::isTimerActive( int id ){
	return timers[id] > 0;
}
bool NetwEsp::isTimerInactive( int id ){
	return timers[id] == 0;
}
void NetwEsp::timerOff( int id ){
	timers[id]=0;
}

void NetwEsp::nextTimerMillis( int id, unsigned long periode){

	if(periode<0) periode=0;
	timers[id] = millis() + periode;
	if(timers[id]==0) timers[id]=1;
}

 

void NetwEsp::trace(const char* id)
{
	Serial.print(F("@"));
	Serial.print(millis()/1000);
	Serial.print(F(" "));Serial.print(id);
	Serial.print(F(" ip="));	 Serial.print( WiFi.localIP());

	Serial.print(F(", nodeId=")); Serial.print( nodeId  );
	Serial.print(F(", wifi:"));
		if(  WiFi.status() == 3) {
			Serial.print(F("OK"));
		} else {
			Serial.print( WiFi.status());Serial.print(F("!!"));
		}
	Serial.print(F(", host=")); Serial.print( WiFi.hostname()   );
	Serial.print(F(", station=")); Serial.print( WiFi.softAPgetStationNum()   );
	Serial.print(F(":")); Serial.print( WiFi.softAPIP()   );
	Serial.print(F(", mode=")); Serial.print( mode  );
	Serial.print(F(", getMode=")); Serial.print( WiFi.getMode()   );


	Serial.println();

	Serial.print(F(" conn=")); Serial.print( client.connected()   );
	Serial.print(F("@")); Serial.print( timers[ESP_CLIENT_CONNECT_TIMER]/1000   );
	Serial.print(F(", web=")); Serial.print( WebStarted   );
 
	Serial.print(F(", reConn=")); Serial.print( connectCount   );
	Serial.print(F(", resetCnt=")); Serial.print( resetConnCount   );
	Serial.print(F("@")); Serial.print( timers[ESP_CLIENT_RESET_TIMER]/1000   );
	Serial.print(F(", up=")); Serial.print( String(upQueue)  );
	Serial.print(F(", svc=")); Serial.print( String(svc)  );
	Serial.print(F(":")); Serial.print( port  );


	Serial.println();

	Serial.flush();
}