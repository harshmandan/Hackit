#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
//convert strings to char* for memory saving
const int httpPort = 80;
const char* ssid = "Soul";		//WiFi_ssid
const char* password = "password@1234";		//WiFi_password
const char* host = "esphome.16mb.com";		// Domain  
String switchpath = "/jsontest.php";   //Path to JSON (Settings)
const int pin = 2;					//GPIO2
String timefunc = "/currtime.php";
String currtime = "/newtime.json";
String datepath = "/lastd.json";	//Path to JSON (Lastupdate)
int mode = 0;
int ch = 0;
int cm =0;
int onisam = 0;
int offisam = 0;
String schd;
int timec = 0;
int onh = 0;
int onm = 0;
int offh = 0;
int offm = 0;
int lastupdate=0;
int tm = 0;
int th=0;
time_t t ; // time variable t
String sw;
WiFiClient client;

int checkupdate()
{
while(1)
{	
	if (!client.connect(host, httpPort))
	{
		Serial.println("connection failed");
		continue;
	}
	Serial.print("\nin function checkupdate...");
	client.print(String("GET ") + datepath + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: keep-alive\r\n\r\n");
	delay(500); // wait for server to respond
	// read response
	String section="header";
	while(client.available())
	{
    String temp;
		String line = client.readStringUntil('\r');
		// we’ll parse the HTML body here
		if (section=="header") 
		{ 
			// headers..
			Serial.print(".");
			if (line=="\n") 
			{ 
				// skips the empty space at the beginning 
				section="json";
			}
		}
		else if (section=="json") 
		{ 
			// print the good stuff
			section="ignore";
			String result = line.substring(1);
			// Parse JSON
			int size = result.length() + 1;
			char json[size];
			result.toCharArray(json, size);
			StaticJsonBuffer<200> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(json);
			if (!root.success())
			{
				Serial.println("parseObject() failed");
				continue;
				//return;
			}
      temp=root["lastupdated"].asString();
			int change = temp.toInt();
			if (change==lastupdate)
			{
				Serial.print("\nexiting function checkupdate...");
				return 1;
			}
			else
			{			
				lastupdate=change;
				Serial.print("\nexiting function checkupdate...");
				return 0;
			}
			
}			
	}
	}
}

void turnon()
{
	Serial.print("\nexecuting function turnon..");
	digitalWrite(pin, LOW); 
	Serial.println("SWITCHED ON!");
}

void turnoff()
{	
	Serial.print("\nexecuting function turnoff..");
	digitalWrite(pin, HIGH);
	Serial.println("SWITCHED OFF!");
}

void timer_count()
{	Serial.print("\nin fucntion timer...");
	while(checkupdate()!=0)
	{
	if((hour()>=th)&&(minute()>=tm))
		{
		if(sw=="on")
			turnon();
		else
			turnoff();		//only implemented turn off! (use scheduler otherwise!)
		break;
		}
	
	Serial.println("5s delay in count! . . . ");
	delay(5000);
	}
	Serial.print("\nexiting function timer");
}

void getsettings()
{
  String temp;
  ch=0;
  cm=0;
  while(1)
  {	if (!client.connect(host, httpPort))
	{
		Serial.println("connection failed");
		continue;
	}
	Serial.print("\nin fucntion getsettings");
	client.print(String("GET ") + switchpath + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: keep-alive\r\n\r\n");
	delay(500); // wait for server to respond
	// read response
	String section="header";
	while(client.available())
	{
		String line = client.readStringUntil('\r');
		// Serial.print(line);
		// we’ll parse the HTML body here
		if (section=="header")
		{
			// headers..
			Serial.print(".");
			if (line=="\n")
			{
				// skips the empty space at the beginning 
				section="json";
			}
		}
		else if (section=="json") 
		{  
			// print the good stuff
			section="ignore";
			String result = line.substring(1);
			// Parse JSON
			int size = result.length() + 1;
			char json[size];
			result.toCharArray(json, size);
			StaticJsonBuffer<200> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(json);
			if (!root.success())
			{
				Serial.println("parseObject() failed");
				continue;
				//return;
			}
			else
			{
			sw=root["switch"].asString();
			if(sw=="on")
				turnon();
			else
				turnoff();
			if(strcmp(root["timer"], "0") != 0)
			{
				temp=root["timer"].asString();
				timec = temp.toInt();
			}
			if(strcmp(root["schd"],"on") == 0)
			{
				temp=root["onh"].asString();
				onh = temp.toInt();
				temp=root["onm"].asString();
				onm = temp.toInt();
				temp=root["offh"].asString();
				offh = temp.toInt();
				temp=root["offm"].asString();
				offm = temp.toInt();	
				onisam=(onh>11)?0:1;   // if am (<12) set to 1
				offisam=(offh>11)?0:1;   // if am (<12) set to 1
			}
			if((timec == 0)&& (strcmp(root["schd"],"off")==0))
			{	
				mode=0;
			}
			if((timec != 0)&& (strcmp(root["schd"],"off")==0))
			{
				mode=1;
			}
			if((timec == 0)&& (strcmp(root["schd"],"on")==0))
			{
				mode=2;
			}
			if((timec != 0)&& (strcmp(root["schd"],"on")==0))
			{	
				mode=3;
			}
			while(timec!=0)
			{
			if (timec<=59)
			{ 
				cm+=timec;
				timec = 0;
			}
			else
			{
				ch++;
				timec-=60;
			}
			}
			Serial.print("ch and cm are : ");
			Serial.print(ch);
			Serial.print(":");
			Serial.print(cm);
			th=hour();
			tm=minute();
			th=((ch+th)>23)?(24-(ch+th)):(ch+th);
			tm=((tm+cm)>59)?(60-(cm+tm)):(cm+tm);
			Serial.print("closing connection. ");
			return;
			}
		}
	}
	}
	Serial.print("\nexiting function getsettings");
}

void updatejsontime()
{	
	while(1)
	{
	if (!client.connect(host, httpPort))
	{
		Serial.println("connection failed");
		continue;
	}
	Serial.print("\nin function updatejsontime");
	client.print(String("GET ") + timefunc + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: keep-alive\r\n\r\n");
	Serial.print("\nexiting function updatejsontime\n");
	return;
	}
	}
	
void settime()
{
while(1)
{
if (!client.connect(host, httpPort))
	{
		Serial.println("connection failed");
		continue;
	}
Serial.print("\nin function settime");
client.print(String("GET ") + currtime + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: keep-alive\r\n\r\n");
	delay(500); // wait for server to respond
	// read response
	String section="header";
	while(client.available())
	{
		String line = client.readStringUntil('\r');
		// Serial.print(line);
		// we’ll parse the HTML body here
		if (section=="header")
		{
			// headers..
			Serial.print(".");
			if (line=="\n")
			{
				// skips the empty space at the beginning 
				section="json";
			}
		}
		else if (section=="json") 
		{  
			// print the good stuff
			section="ignore";
			String result = line.substring(1);
			// Parse JSON
			int size = result.length() + 1;
			char json[size];
			result.toCharArray(json, size);
			StaticJsonBuffer<200> jsonBuffer;
			JsonObject& root = jsonBuffer.parseObject(json);
			if (!root.success())
			{
				Serial.println("parseObject() failed");
				Serial.println("continuing . . . after 5s . . .");
				delay(5000);
				continue;
				//return;
			}
			else
			{
				String h,m,s,d,mon,y;
				h=root["h"].asString();
				m=root["m"].asString();
				s=root["s"].asString();
				d=root["d"].asString();
				mon=root["mon"].asString();
				y=root["y"].asString();
				setTime(h.toInt(),m.toInt(),0,d.toInt(),mon.toInt(),y.toInt()); // yr is 2 or 4 digit yr; second is set to 0 permanently, beacuse timer doesnt check for seconds
				t=now();
				Serial.print("\nnew time is :");
				Serial.print(hour());
				Serial.print(":");
				Serial.print(minute());
				return;
			}
		
		}
	}
	Serial.print("closing connection. ");
	}
		Serial.print("\nexiting function settime");

}

void setup() 
{  
	pinMode(pin, OUTPUT); 
	pinMode(pin, HIGH);
	Serial.begin(115200);
	delay(10);
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);
	int wifi_ctr = 0;
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("WiFi connected");  
	Serial.println("IP address: " + WiFi.localIP());
}

void loop()
{
	Serial.print("entered loop... connecting to ");
	Serial.println(host);
	Serial.print("Set time is: ");
	Serial.print(hour());
	Serial.print(":");
	Serial.print(minute());
	if (!client.connect(host, httpPort))
	{
		Serial.println("connection failed");
		return;
	}
	if (checkupdate() == 0)
	{
		delay(1000);
		updatejsontime();
		delay(1000);
		settime();
		delay(1000);
		getsettings();
	}
	switch (mode)
	{
		case 0:
			Serial.print("..case 0...");
			if(sw=="on")
				turnon();
			else
				turnoff();
			break;
		case 1:
			Serial.print("..case 1...");
			timer_count();
			break;
		case 2:
			Serial.print("..case 2 . . .");
			Serial.println(isAM());
      Serial.println(onisam);
      Serial.println(offisam);
			if ((hour() >= offh) && (isAM()==offisam) && (minute()>= offm))
				turnoff();							//check for off first or it won't work!
			else if((hour() >= onh) && (isAM()==onisam) && (minute() >= onm))
				turnon();
			break;
		case 3:
			Serial.print("..case 3...");
			timer_count();			/* First call timer, then check for scheduling
								On the application layer, the timer will not be
								allowed to overlap the scheduler in any case */
			if (hour() >= offh && isAM()==offisam && minute()>= offm)
				turnoff();							//check for off first or it won't work!
			else if(hour() >= onh && isAM()==onisam && minute() >= onm)
				turnon();
		
			break;
		default:
				Serial.print("..ERROR...default case..");
				lastupdate=0;  //Set it to 0, so it can update
			break;
	}
	delay(5000);
	Serial.println("DELAY5s");
}
