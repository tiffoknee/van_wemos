
//
// Van Automation
//

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>

const int OFF = 0;
const int ON = 1;
const int AUTO = 2;

#define ledPin D5
#define dhtIntInPin D7
#define dhtIntOutPin D1
#define dhtExtOutPin D2
#define dhtExtInPin D3 //define DHT pins for all four sensors on heat exchanger
#define DHTTYPE DHT11    //define the sensor used(DHT11)
#define fanPin D6 //run both fans off this pin
#define fanButtonPin D4 // momentary press button, state machine
#define ledButtonPin D1 // press button, state machine with hold

int fanState = OFF; // 0 == off, 1 == on, 2 == auto

unsigned long ledButtonMillis = 0;
unsigned long dhtLastCheck = 0;
unsigned long fanButtonLastPressed = 0;

long dhtInterval = 6000;           // milliseconds between sensor checks

float maxTemp = 22.00;
float maxHumidity = 80.00;


DHT dhtIntIn(dhtIntInPin, DHTTYPE);//create an instance of DHT
DHT dhtExtIn(dhtExtInPin, DHTTYPE);//create an instance of DHT
DHT dhtIntOut(dhtIntOutPin, DHTTYPE);//create an instance of DHT
DHT dhtExtOut(dhtExtOutPin, DHTTYPE);//create an instance of DHT

const char* ssid = "Workshop";             //!!!!!!!!!!!!!!!!!!!!! modify this
const char* password = "bananabanana";                //!!!!!!!!!!!!!!!!!!!!!modify this
 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(6, ledPin, NEO_GRB + NEO_KHZ800);
 
WiFiServer server(80);
 
void setup() {

  
  Serial.begin(115200);
  delay(10);
  
  pinMode(fanPin, OUTPUT);
  pinMode(fanButtonPin, INPUT);    // declare pushbutton as input
  pinMode(ledButtonPin, INPUT);    // declare pushbutton as input
  
  digitalWrite(fanPin, LOW);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
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

  // end wifi setup 

  //DHT setup
  delay(6000);           //wait 6 seconds
  Serial.println("Temperature and Humidity test!");//print on Serial monitor
  Serial.println("T(C) \tH(%)");  //print on Serial monitor
  
  dhtIntIn.begin();
  dhtExtIn.begin();
  dhtIntOut.begin();
  dhtExtOut.begin();          //initialize the Serial communication
  
  //end DHT setup

  // LEDs
  pixels.begin(); // This initializes the NeoPixel library.
  // END LEDS

//  pinMode(fanPin, INPUT);

}

 
void loop() {

  unsigned long currentMillis = millis();

  val = digitalRead(fanButtonPin);  // read fan button
  
  if (val == LOW && (currentMillis - fanButtonLastPressed >= 100) {  // check if the input is LOW (button pressed) and debounce
    
    fanButtonLastPressed = currentMillis;
    //is the button momentary click? I think so!
    
    if(fanState == OFF){
      fanState = ON;
      digitalWrite(fanPin, HIGH);     
    }

    if(fanState == ON){
      fanState = AUTO;   
    }

    if(fanState == AUTO){
      fanState = OFF;
      digitalWrite(fanPin, LOW);
    }
  }

int led;
  for(led=0; led <=6; led++)
  {
    setColor(led,255,0,0,100); //red
  }
  for(led=0; led <=6; led++)
  {
    setColor(led,0,255,255,100); //
  }
    for(led=0; led <=6; led++)
  {
    setColor(led,255,0,255,100); //
  }
  
  if((currentMillis - dhtLastCheck >= dhtInterval) ||
  (fanButtonLastPressed == currentMillis && fanState == AUTO){  //check every dhtInterval milliseconds OR if fan is set to auto

    float h = dhtIntIn.readHumidity();    // reading Humidity 
    float t = dhtIntIn.readTemperature(); // read Temperature as Celsius (the default)
    // check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {    
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.print(t, 2);    //print the temperature
    Serial.print("\t");
    Serial.println(h, 2);  //print the humidity

    if(fanState == AUTO){ //if fan is set to auto, check the temp and humidity
      if((t >= maxTemp || h >= maxHumidity){
        digitalWrite(fanPin, HIGH);        
      }else{
        digitalWrite(fanPin, LOW);        
      }
    }
     
    
    dhtLastCheck = currentMillis; //reset dhtLastCheck
  }


  

   
  if (request.indexOf("/FAN=ON") != -1) {
    digitalWrite(fanPin, HIGH);
    fanState = ON;
  } 
  if (request.indexOf("/FAN=OFF") != -1){
    digitalWrite(fanPin, LOW);
    fanState = OFF;
  }
   if (request.indexOf("/FAN=AUTO") != -1){   
    fanState = AUTO;
  }
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
 
  

//  if (request.indexOf("/FAN=") != -1){
//
//    request.remove(0,5);
//    analogWrite(fanPin,request.toInt());
//
//    fanSpeedString = request;
//     
//  }

 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("Fan pin is now: ");
 
  if(FANvalue == HIGH) {
    client.print("On");  
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
