
/*
  Arduino egg hatcher

  Parts required:
  - one TMP36 temperature sensor
  - 1 red LEDs
  - 1 220 ohm resistor
  - Velleman VMA311 (DHT11)
  - Velleman 4 channel relay module VMA400

  created 9 februari 2019
  by Koen Mestdag

  This example code is part of the public domain.
*/

// named constant for the pin the sensor is connected to
const int sensorPin = A0;
// lightbulbindex
const int LIGHT_BULB_INDEX = 2;
// room temperature in Celsius
const float targetTemp = 26.7;
// Fix the "floating" temperature reading
float averagetemp = 162;
float temps[25];
float dhtTemp;

// Set up the LCD
// include the LCD code library:
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

// Set up the temp & moisture
#include <dht.h>
dht DHT;
#define DHT11_PIN 7

// set up servo
#include <Servo.h>
Servo myServo;
int const potPin = A1;
int potVal;
int angle;

// variable to know when to tilt!
int startTime = 0;
int elapsedTime = 0;

void setup() {
  // open a serial connection to display values on computer
  Serial.begin(9600);
  
  // set the LED pins as outputs
  pinMode(LIGHT_BULB_INDEX, OUTPUT);
  // turn LAMP ON
  digitalWrite(LIGHT_BULB_INDEX, HIGH);

  // start up the display
  // set up the number of columns and rows on the LCD
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("STARTING");
  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);
  lcd.print("EGG BREADING");

  // initialize averagetemp
  int i;
  for (i = 0; i < 25; i++) {
    temps[i] = averagetemp;
  }

  // setup servo
  myServo.attach(9);
  
  // var to keep track of tilting
  startTime = millis();
  elapsedTime = 0;

  // start with clear messages on the LCD
  Serial.print("STARTING WITH TARGET T = ");
  Serial.println(targetTemp);
  // show the message
  delay(500);
  Serial.println("STARTED");
  // show the message
  delay(500);
}

void loop() {  
  
  // read the value on AnalogIn pin 0 (temp sensor) and store it in a variable
  int sensorVal = analogRead(sensorPin);

  // temperature varies to much: calculate average!
  int i;
  averagetemp = 0;
  for (i = 24; i > 0; i--) {
    averagetemp = averagetemp + temps[i];
    temps[i] = temps[i - 1];
  }
  temps[0] = sensorVal;
  averagetemp = averagetemp + temps[0];
  averagetemp = averagetemp / 25;

  // convert the ADC reading to voltage
  float voltage = (averagetemp / 1024.0) * 5.0;

  // convert the voltage to temperature in degrees C
  // the sensor changes 10 mV per degree
  // the datasheet says there's a 500 mV offset
  // ((voltage - 500 mV) times 100)
  float temperature = (voltage - .5) * 100;

  Serial.print(" AVG[");
  Serial.print(averagetemp);
  Serial.print("]");
  Serial.print("Target[");
  Serial.print(targetTemp);
  Serial.print("] Actual[");
  Serial.print(temperature);
  Serial.println("]");

  // DISPLAY
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("t");
  lcd.print(targetTemp);
  lcd.print(" act");
  lcd.print(temperature);

  int chk = DHT.read11(DHT11_PIN);
  dhtTemp = DHT.temperature;
  Serial.print("Temperature = ");
  Serial.print(dhtTemp);
  Serial.print(" Humidity = ");
  Serial.println(DHT.humidity);

  lcd.setCursor(0, 1);
  lcd.print("T");
  lcd.print(DHT.temperature);
//  lcd.print((char)223);
//  lcd.print("C");
  lcd.print(" M");
  lcd.print(DHT.humidity);
  lcd.println("%");

  // TURN LIGHT ON / OFF TO MANIPULATE TEMPERATURE
  if(dhtTemp > 1) {
    // correct reading, proceed
    if (dhtTemp < targetTemp - 0.5) { // + 2) {
        // too cold: turn light on
        digitalWrite(LIGHT_BULB_INDEX, HIGH);
    } else if (dhtTemp > targetTemp + 0.2) {
        // too warm: turn light off
        digitalWrite(LIGHT_BULB_INDEX, LOW);
    } else {
        // keep lamp on or off, temp is OK
    }
  }
  // delay to give temp and moist time to read
  delay(1000);

  // turn the eggs every 8 hour! = 3600s * 8
  // calculate the time sinds last check
  elapsedTime = millis() - startTime;
  if(elapsedTime >= 10000) {
    startTime = millis();
    //// read the value of the potentio switch
    //potVal = analogRead(potPin);
    //// change the 1023 bits to 360 degrees
    //angle = map(potVal, 0, 1023, 0, 179);
    if (angle == 0) {
      angle = 150;
    } else {
      angle = 30;
    };
    // change the servo motor to the position of the potentio switch
    myServo.write(angle);
    Serial.print("EGGS WERE TURNED TO ANGLE ");
    Serial.print(angle);
    Serial.print(" at ");
    Serial.print((millis() / 1000));
    Serial.println("s");
  }



  // wait for x * 1000ms so that the lamps do not keep going on and off
  delay(100);
}
