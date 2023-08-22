#include <SoftwareSerial.h>

#include <SomeSerial.h>

#include "DHT.h"
// dht sensor by adafruit should be installed
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SoftwareSerial.h>

unsigned long loopdelay_ms = 30000;
unsigned long looptime;

#define DHTPIN 2     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

#define SCL 8 // SCL connect pin
#define SDA 9 // SDA connect pin
#define CSB 10 // CSB connect pin
#define SDO 11 // SDO connect pin

Adafruit_BMP280 bmp(CSB, SDA, SDO, SCL);

/* Grove - Dust Sensor Demo v1.0
  Interface to Shinyei Model PPD42NS Particle Sensor
  Program by Christopher Nafis
  Written April 2012
  http://www.seeedstudio.com/depot/grove-dust-sensor-p-1050.html
  http://www.sca-shinyei.com/pdf/PPD42NS.pdf
  JST Pin 1 (Black Wire) => Arduino GND
  JST Pin 3 (Red wire) => Arduino 5VDC
  JST Pin 4 (purple wire) => Arduino Digital Pin 13
*/
int pin = 13;
unsigned long duration;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float ugm3 = 0;

// hc 06 bluetooth
#define BT_RXD 6
#define BT_TXD 7
SoftwareSerial bluetooth(BT_TXD, BT_RXD);

// SoftwareSerial 2ea installed

void setup() {
  Serial.begin(9600);
  Serial.println("temp, Humi, BMP");

  dht.begin();

  if ( !bmp.begin()) {
    Serial.println("No connect to sensor BMP. please checkup sensor connection status.");
  }

  // micro dust setup
  pinMode(pin, INPUT);
  looptime = millis(); // loop check time;

  // bluetooth setup hc-06
  bluetooth.begin(9600);
}

void loop() {
  // Wait a few seconds between measurements.
  duration = pulseIn(pin, LOW);
    lowpulseoccupancy = lowpulseoccupancy + duration;
  if ((millis() - looptime) > loopdelay_ms ) //
  {

    int temp = (int)dht.readTemperature();
    int humi = (int)dht.readHumidity();



    //  1h.     4h.     7h.     10h.    13h.    16h.    19h.    22h
    // 1003.6	1003.7	1004.7	1005.2	1003.9	1003.0	1003.5	1005.2
    // https://www.weather.go.kr/w/obs-climate/land/city-obs.do?auto_man=m&stn=0&dtm=&type=t08&reg=109&tm=2023.08.17.23%3A00


    ratio = lowpulseoccupancy / (loopdelay_ms * 10.0); // Integer percentage 0=>100
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve
    ugm3 = concentration * 100 / 13000;

    float bmpTemperature = bmp.readTemperature();
    float bmpPressure = bmp.readPressure();
    float bmpAltitude = bmp.readAltitude(1005.2);

    Serial.print("Temperature = ");
    Serial.print(temp); //온도값 시리얼 모니터에 출력
    Serial.println();
    Serial.print("Humidity = ");
    Serial.println(humi); //습도: 출력

    Serial.print("BMPTemperature = ");
    Serial.println(bmpTemperature);
    Serial.print("BMPPressure = ");
    Serial.println(bmpPressure);
    Serial.print("BMPAltitude = ");
    Serial.println(bmpAltitude);

    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    Serial.print(ugm3);
    Serial.println(" ug/m3 [PM1.0]");

    String output = String(temp) + ","
                    + String(humi) + ","
                    + String(bmpTemperature) + ","
                    + String(bmpPressure) + ","
                    + String(bmpAltitude) + ","
                    + String(concentration) + ","
                    + String(ugm3);


    Serial.println(output);
    Serial.println("\n");
    bluetooth.println(output);

    lowpulseoccupancy = 0;
    looptime = millis();
  }

  if (bluetooth.available()) {
    Serial.write(bluetooth.read());
  }


}


// refer site
// dht : https://fishpoint.tistory.com/7080
// BMP : https://m.blog.naver.com/eduino/222066669634
// micro dust : https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=pa3018&logNo=221563499384
