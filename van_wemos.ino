
//
// Van Automation
//

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <knownwifi.h>

////define all the known wifi networks I want it to try and connect to for dumping data
//This is defined in knownwifi.h as per below, and saved in Arduino/Libraries/MyCommon
//const char* knownWifi[][2] = {{"SSID1", "password1"},
//  {"SSID2", "password2"},
//  {"SSID3", "password3"}
//};

//int noKnown = 3;

const int OFF = 0;
const int ON = 1;
const int AUTO = 2;

#define ledPin D6
#define dhtIntInPin D4
#define dhtIntOutPin D3
#define dhtExtInPin D2 //define DHT pins for all four sensors on heat exchanger
#define dhtExtOutPin D1
#define DHTTYPE DHT11    //define the sensor used(DHT11)
#define inFanPin D5
#define outFanPin D0
#define fanButtonPin D8 // momentary press button, state machine
#define ledButtonPin D7 // press button, state machine with hold

int fanState = OFF; // 0 == off, 1 == on, 2 == auto

unsigned long ledButtonMillis = 0;
unsigned long dhtLastCheck = 0;
unsigned long fanButtonLastPressed = 0;
unsigned long wifiLastCheck = 0;

long dhtInterval = 6000;           // milliseconds between sensor checks
long checkForWifiInterval = 6000;  //milliseconds between checks for wifi

float maxTemp = 23.00;
float maxHumidity = 80.00;
int led;

DHT dhtIntIn(dhtIntInPin, DHTTYPE);//create an instance of DHT
DHT dhtExtIn(dhtExtInPin, DHTTYPE);//create an instance of DHT
DHT dhtIntOut(dhtIntOutPin, DHTTYPE);//create an instance of DHT
DHT dhtExtOut(dhtExtOutPin, DHTTYPE);//create an instance of DHT

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(6, ledPin, NEO_GRB + NEO_KHZ800);

WiFiServer server(80);

WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);


void setup() {
  timeClient.begin();

  Serial.begin(115200);
  delay(10);

  pinMode(inFanPin, OUTPUT);
  pinMode(outFanPin, OUTPUT);
  pinMode(fanButtonPin, INPUT);    // declare pushbutton as input
  pinMode(ledButtonPin, INPUT);    // declare pushbutton as input

  digitalWrite(inFanPin, LOW);
  digitalWrite(outFanPin, LOW);

  //  // Connect to WiFi network
  joinWifi();

  // end wifi setup

  //DHT setup
  delay(6000);           //wait 6 seconds
  Serial.println("Temperature and Humidity test!");//print on Serial monitor
  Serial.println("T(C) \tH(%)");  //print on Serial monitor

  dhtIntIn.begin();
  dhtExtIn.begin();
  dhtIntOut.begin();
  dhtExtOut.begin();          //.. initialize the Serial communication
  //end DHT setup

  // LEDs
  pixels.begin(); // This initializes the NeoPixel library.
  // END LEDS

  

}


void loop() {


  unsigned long currentMillis = millis();
  int val;

  //check and see if there's available wifi..
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - wifiLastCheck >= checkForWifiInterval)) {
    joinWifi();
    wifiLastCheck = currentMillis;
  }

  val = digitalRead(fanButtonPin);  // read fan button

  if (val == HIGH && (currentMillis - fanButtonLastPressed >= 600)) {  // check if the input is LOW (button pressed) and debounce

    fanButtonLastPressed = currentMillis;
    //is the button momentary click? I think so!

    if (fanState == OFF) {
      fanState = ON;
      digitalWrite(inFanPin, HIGH);
      digitalWrite(outFanPin, HIGH);
      Serial.println("FAN - ON");
      for (led = 0; led <= 6; led++)
      {
        setColor(led, 0, 255, 0, 100); //green
      }

    }

    else if (fanState == ON) {
      fanState = AUTO;
      Serial.println("FAN - AUTO");
      for (led = 0; led <= 6; led++)
      {
        setColor(led, 0, 0, 255, 100); //blue
      }

    }

    else if (fanState == AUTO) {
      fanState = OFF;
      digitalWrite(inFanPin, LOW);
      digitalWrite(outFanPin, LOW);
      Serial.println("FAN - OFF");
      for (led = 0; led <= 6; led++)
      {
        setColor(led, 255, 0, 0, 100); //blue
      }

    }
  }


  if ((currentMillis - dhtLastCheck >= dhtInterval) ||
      (fanButtonLastPressed == currentMillis && fanState == AUTO)) { //check every dhtInterval milliseconds OR if fan is set to auto

    float h = dhtIntIn.readHumidity();    // reading Humidity
    float t = dhtIntIn.readTemperature(); // read Temperature as Celsius (the default)
    // check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      // Serial.println("Failed to read from DHT 1 sensor!");
      return;
    } else {

      Serial.print(t, 2);    //print the temperature
      Serial.print("\t");
      Serial.println(h, 2);  //print the humidity
    }
    h = dhtIntOut.readHumidity();    // reading Humidity
    t = dhtIntOut.readTemperature(); // read Temperature as Celsius (the default)
    // check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      // Serial.println("Failed to read from DHT 1 sensor!");
      return;
    } else {

      Serial.print(t, 2);    //print the temperature
      Serial.print("\t");
      Serial.println(h, 2);  //print the humidity
    }
    h = dhtExtIn.readHumidity();    // reading Humidity
    t = dhtExtIn.readTemperature(); // read Temperature as Celsius (the default)
    // check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      // Serial.println("Failed to read from DHT 1 sensor!");
      return;
    } else {

      Serial.print(t, 2);    //print the temperature
      Serial.print("\t");
      Serial.println(h, 2);  //print the humidity
    }
    h = dhtExtOut.readHumidity();    // reading Humidity
    t = dhtExtOut.readTemperature(); // read Temperature as Celsius (the default)
    // check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      // Serial.println("Failed to read from DHT 1 sensor!");
      return;
    } else {

      Serial.print(t, 2);    //print the temperature
      Serial.print("\t");
      Serial.println(h, 2);  //print the humidity
    }
    if (fanState == AUTO) { //if fan is set to auto, check the temp and humidity
      if ((t >= maxTemp || h >= maxHumidity)) {
        digitalWrite(inFanPin, HIGH);
        digitalWrite(outFanPin, HIGH);
        Serial.println("auto - ON");
      } else {
        digitalWrite(inFanPin, LOW);
        digitalWrite(outFanPin, LOW);
        Serial.println("auto - OFF");
      }
    }


    dhtLastCheck = currentMillis; //reset dhtLastCheck
  }




  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();


  if (request.indexOf("/FAN=ON") != -1) {
    digitalWrite(inFanPin, HIGH);
    digitalWrite(outFanPin, HIGH);
    fanState = ON;
  }
  if (request.indexOf("/FAN=OFF") != -1) {
    digitalWrite(inFanPin, LOW);
    digitalWrite(outFanPin, LOW);
    fanState = OFF;
  }
  if (request.indexOf("/FAN=AUTO") != -1) {
    fanState = AUTO;
  }

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("Fan state is now: ");

  if (fanState == ON) {
    client.print("On");
  } else if (fanState == OFF) {
    client.print("Off");
  } else {
    client.print("Off");
  }

  client.println("<br><br>");
  client.println("Click <a href=\"/FAN=ON\">here</a> turn the FAN on pin 4 ON<br>");
  client.println("Click <a href=\"/FAN=OFF\">here</a> turn the FAN on pin 4 OFF<br>");
  client.println("</html>");

  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");

}

//simple function which takes values for the red, green and blue led and also
//a delay
void setColor(int led, int redValue, int greenValue, int blueValue, int delayValue)
{
  pixels.setPixelColor(led, pixels.Color(redValue, greenValue, blueValue));
  pixels.show();
  delay(delayValue);
}

void joinWifi() {


  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();


  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    for (int j = 0; j < noKnown; ++j) {
      if (WiFi.SSID(i) == knownWifi[j][0]) {

        Serial.print("Connecting to ");
        Serial.println(knownWifi[j][0]);

        WiFi.mode(WIFI_STA);
        WiFi.begin(knownWifi[j][0], knownWifi[j][1]);

        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi connected");

        // Start the server
        server.begin();
        Serial.println("Server started");

        // Print the IP address
        Serial.print("Use this URL : ");
        Serial.print("http://");
        Serial.print(WiFi.localIP());
        Serial.println("/");

        timeClient.update();
        Serial.println(timeClient.getFormattedTime());

        return;

      }
    }
  }
}

