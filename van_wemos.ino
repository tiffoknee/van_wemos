
//
// Van Automation
//

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>

#define ledPin D5
#define dhtPin D7        //define as DHTPIN the Pin 3 used to connect the Sensor
#define DHTTYPE DHT11    //define the sensor used(DHT11)
#define fanPin D6

unsigned long ledMillis = 0;
unsigned long dhtLastCheck = 0;

long dhtInterval = 6000;           // milliseconds between sensor checks


DHT dht(dhtPin, DHTTYPE);//create an instance of DHT

const char* ssid = "Workshop";             //!!!!!!!!!!!!!!!!!!!!! modify this
const char* password = "bananabanana";                //!!!!!!!!!!!!!!!!!!!!!modify this
 
//int fanPin = D5;          

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(6, ledPin, NEO_GRB + NEO_KHZ800);

  
WiFiServer server(80);
 
void setup() {

  
  Serial.begin(115200);
  delay(10);
  
  pinMode(fanPin, OUTPUT);
//  pinMode(ledPin, OUTPUT);
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
  
  delay(6000);           //wait 6 seconds
  Serial.println("Temperature and Humidity test!");//print on Serial monitor
  Serial.println("T(C) \tH(%)");                   //print on Serial monitor
  dht.begin();           //initialize the Serial communication

  pixels.begin(); // This initializes the NeoPixel library.
  

 

//  pinMode(fanPin, INPUT);


 


}

 
void loop() {

  unsigned long currentMillis = millis();
  

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
  
  if(currentMillis - dhtLastCheck >= dhtInterval){  //check every dhtInterval milliseconds

    float h = dht.readHumidity();    // reading Humidity 
    float t = dht.readTemperature(); // read Temperature as Celsius (the default)
    // check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {    
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.print(t, 2);    //print the temperature
    Serial.print("\t");
    Serial.println(h, 2);  //print the humidity

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
 
  // Match the request
 
  int FANvalue = LOW;
  
  
  if (request.indexOf("/FAN=ON") != -1) {
    digitalWrite(fanPin, HIGH);
    FANvalue = HIGH;
  } 
  if (request.indexOf("/FAN=OFF") != -1){
    digitalWrite(fanPin, LOW);
    FANvalue = LOW;
  }

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
