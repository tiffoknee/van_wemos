## Campervan Automation

Wemos Pro based solution for recording temperature and humidity data, running fans automatically based on the data, and feeding back with LEDs.  It will record data from 4 DHT11 sensors to track the effect of a DIY heat exchanger, run two 12v pc fans, and a string of 8 WS2821b LEDs.

Note: if the RGBs do not respond, but they work fine on an arduino, update your adafruit_neopixel library. You may have an old version that doesn't support ESP8266.

Note to self: DHT11 sensors are apparently a bit rubbish and granularity is 1 degree. BME280's are much better. Swap these out at some point.

Next steps: 

- add a threshold temp/humidity at which the fans turn on DONE
- add a button that turns fans on/off/auto DONE
- add a button to cycle LED effects DONE
- make wifi a hotspot ON HOLD
- add known wifi networks DONE
- make LEDs more interesting. respond to button DONE
- return LEDs to current settings after fan feedback DONE

- post data to influxdb - TO TEST
