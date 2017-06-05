
//
// Van Automation
//

#include <SPI.h>
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

const int ledsDOWN = 0;
const int ledsUP  = 1;

int status = WL_IDLE_STATUS;     // the Wifi radio's status

// the IP address of the InfluxDB host
byte host[] = {88, 80, 191, 61};

// the port that the InfluxDB UDP plugin is listening on
int port = 8888;

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

int currentR = 127;
int currentG = 127;
int currentB = 127;
int fanControlState = AUTO; // 0 == off, 1 == on, 2 == auto
int fanState = AUTO;

int activeLEDs = 0; //number of LEDs currently active
int secondCount = 0; //don't change the active led if it's already been changed this second

int showColour = 0;
unsigned long dhtLastCheck = 0;
unsigned long fanButtonLastPressed = 0;
unsigned long wifiLastCheck = 0;

int prevLEDs = ledsDOWN; // 0 = previous LEDs ledsDOWN, 1 = previous LEDs ledsUP

int current;         // Current state of the button
// (LOW is pressed b/c i'm using the pullup resistors)
long millis_held;    // How long the button was held (milliseconds)
long secs_held;      // How long the button was held (seconds)
long prev_secs_held; // How long the button was held in the previous check
byte previous = HIGH;
unsigned long firstTime; // how long since the button was first pressed

long dhtInterval = 30000;           // milliseconds between sensor checks
long checkForWifiInterval = 30000;  //milliseconds between checks for wifi

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
  //  Serial.println("Temperature and Humidity test!");//print on Serial monitor
  //  Serial.println("T(C) \tH(%)");  //print on Serial monitor

  dhtIntIn.begin();
  dhtExtIn.begin();
  dhtIntOut.begin();
  dhtExtOut.begin();          //.. initialize the Serial communication

  //end DHT setup

  // LEDs
  pixels.begin(); // This initializes the NeoPixel library.

  pixels.show();

  // END LEDS



}


void loop() {



  String lineSend = "";
  unsigned long currentMillis = millis();
  int val;
  String now = String(timeClient.getEpochTime());

  //handle the LED button

  current = digitalRead(ledButtonPin);

  // if the button state changes to pressed, remember the start time
  if (current == HIGH && previous == LOW && (millis() - firstTime) > 200) {
    firstTime = millis();
  }

  millis_held = (millis() - firstTime);
  secs_held = millis_held / 1000;

  // This if statement is a basic debouncing tool, the button must be pushed for at least
  // 100 milliseconds in a row for it to be considered as a push.
  if (millis_held > 50) {

    if (current == HIGH && secs_held > prev_secs_held) {
      Serial.print("1 prevLEDs = ");
      Serial.println(prevLEDs);
      if (secondCount != secs_held) {

        if (prevLEDs == ledsUP  && activeLEDs > 0) { //decrease number

          if (activeLEDs != 0) {
            setColor(activeLEDs - 1, 0, 0, 0, 50);
            Serial.println("setColor(" + String(activeLEDs - 1) + "0, 0, 0, 50)");
            pixels.show();
            activeLEDs --;
            Serial.print("decrement active LEDs to:");
            Serial.println(activeLEDs);
            secondCount = secs_held;
          }


        } else { //increase number

          if (activeLEDs == 7) {
            colorWipe(pixels.Color(0, 0, 0), 50, activeLEDs);
            secondCount = secs_held;
            activeLEDs = 0;
            Serial.print("active LEDs = 7. set to 0");
            Serial.println(activeLEDs);
          } else {

            setColor(activeLEDs - 1, currentR, currentG, currentB, 100);

            Serial.println("setColor(" + String(activeLEDs - 1) + ", 100, 255, 150, 100)");

            activeLEDs ++;

            Serial.print("increment active LEDs to:");
            Serial.println(activeLEDs);
            secondCount = secs_held;
          }
        }

      }

      Serial.println();
      Serial.println(String(secs_held) + " + prev" + String(prev_secs_held));


    }

    // check if the button was released since we last checked
    if (current == LOW && previous == HIGH && currentMillis > 19000) {

      if (activeLEDs == 0) {
        prevLEDs = ledsDOWN; //decrease
      } else {
        if (prevLEDs == ledsDOWN) {
          prevLEDs = ledsUP ; //increase
        } else {
          prevLEDs = ledsDOWN ; //increase
        }

      }

      // Button pressed for less than 1 second, change colour
      if (secs_held <= 0) {
        showColour++;
        if (showColour > 4)
          showColour = 0;
        cycleColour(showColour);
         colorWipe(pixels.Color(currentR, currentG, currentB), 5, activeLEDs - 1);
      }


    }
  }

  previous = current;
  prev_secs_held = secs_held;

  //end handle LED button

  if (WiFi.status() == WL_CONNECTED) {
    lineSend += ("epoch_time value=" + now); // build a string to post to the influxdb
    //Serial.println(lineSend);
  }


  //check and see if there's available wifi..
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - wifiLastCheck >= checkForWifiInterval)) {
    joinWifi();
    wifiLastCheck = currentMillis;
  }

  val = digitalRead(fanButtonPin);  // read fan button

  if (val == HIGH && (currentMillis - fanButtonLastPressed >= 600)) {  // check if the input is LOW (button pressed) and debounce


    if (fanControlState == OFF) {
      fanControlState = ON;
      fanState = ON;
      digitalWrite(inFanPin, HIGH);
      digitalWrite(outFanPin, HIGH);
      Serial.println("FAN - ON");
      for (led = 0; led <= 6; led++)
      {
        setColor(led, 0, 255, 0, 100); //green
      }
      colorWipe(pixels.Color(currentR, currentG, currentB), 5, activeLEDs - 1);


    }

    else if (fanControlState == ON) {
      fanControlState = AUTO;
      Serial.println("FAN - AUTO");
      for (led = 0; led <= 6; led++)
      {
        setColor(led, 0, 0, 255, 100); //blue
      }
      colorWipe(pixels.Color(currentR, currentG, currentB), 5, activeLEDs - 1);
    }

    else if (fanControlState == AUTO) {
      fanControlState = OFF;
      fanState = OFF;
      digitalWrite(inFanPin, LOW);
      digitalWrite(outFanPin, LOW);
      Serial.println("FAN - OFF");
      for (led = 0; led <= 6; led++)
      {
        setColor(led, 255, 0, 0, 100); //blue
      }
      colorWipe(pixels.Color(currentR, currentG, currentB), 5, activeLEDs - 1);

    }
    fanButtonLastPressed = currentMillis;
    //is the button momentary click? I think so!
  }


  if ((currentMillis - dhtLastCheck >= dhtInterval) ||
      (fanButtonLastPressed == currentMillis && fanControlState == AUTO)) {
    //check every dhtInterval milliseconds OR if fan is set to auto

    lineSend += checkSensor(dhtIntIn.readHumidity(), dhtIntIn.readTemperature(), "intIn");
    lineSend += checkSensor(dhtIntOut.readHumidity(), dhtIntOut.readTemperature(), "intOut");
    lineSend += checkSensor(dhtExtIn.readHumidity(), dhtExtIn.readTemperature(), "extIn");
    lineSend += checkSensor(dhtExtOut.readHumidity(), dhtExtOut.readTemperature(), "extOut");

    if (fanControlState == AUTO) { //if fan is set to auto, check the temp and humidity
      if ((dhtIntOut.readTemperature() >= maxTemp || dhtIntOut.readHumidity() >= maxHumidity)) {
        digitalWrite(inFanPin, HIGH);
        digitalWrite(outFanPin, HIGH);
        Serial.println("auto - ON");
        fanState = ON;
      } else {
        digitalWrite(inFanPin, LOW);
        digitalWrite(outFanPin, LOW);
        Serial.println("auto - OFF");
        fanState = OFF;
      }
    }




    if (WiFi.status() == WL_CONNECTED) {
      lineSend += ", fans value=" + String(fanState);
      Serial.println(lineSend);


      Serial.println(" send data (disabled)");
      //sendData(lineSend); //send the data
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
    fanControlState = ON;
  }
  if (request.indexOf("/FAN=OFF") != -1) {
    digitalWrite(inFanPin, LOW);
    digitalWrite(outFanPin, LOW);
    fanControlState = OFF;
  }
  if (request.indexOf("/FAN=AUTO") != -1) {
    fanControlState = AUTO;
  }

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");

  client.print("Fan state is now: ");

  if (fanControlState == ON) {
    client.print("On");
  } else if (fanControlState == OFF) {
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

int cycleColour(int i) {
  switch (i) {
    case 0:  // warm white
      currentR = 127;
      currentG = 100;
      currentB = 80;
      break;
    case 1:  // glowy dim
      currentR = 60;
      currentG = 30;
      currentB = 10;
      break;

    case 2:  // Red
      currentR = 180;
      currentG = 0;
      currentB = 0;
      break;

    case 3:   // dark blue
      currentR = 0;
      currentG = 0;
      currentB = 100;
      break;
      

    case 4:   // yellow
      currentR = 200;
      currentG = 100;
      currentB = 0;
      break;

  }
}

// Fill the dots one after the other with a color
void colorWipe(int c, char wait, int n) {
  for (short i = 0; i < n; i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
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

void sendData(String s) {
  // send the packet
  Serial.println("Sending UDP packet...");
  ntpUDP.beginPacket(host, port);
  ntpUDP.print(s);
  ntpUDP.endPacket();
}

String checkSensor(float h, float t, String sensor) {

  // check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    // Serial.println("Failed to read from "+sensor+"sensor!");
    return "";
  } else {
    // concatenate the temperature into the line protocol
    //Serial.println(","+sensor + "Humidity value=" + String(h) + ", "+ sensor +"Temperature value=" + String(t));

    return "," + sensor + "Humidity value=" + String(h) + ", " + sensor + "Temperature value=" + String(t);
  }

}

void rainbow(char wait) {
  short i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
int Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


