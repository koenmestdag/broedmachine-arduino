
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
const float targetTemp = 37.0;
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
// servo object which represents the real thing
Servo myServo;
// the pin that is connected to the servo
int const potPin = A1;
// angle switches
const int leftAngle = 70;
const int rightAngle = 110;
// the current angle
int angle = 90;
// step by step rotation (local var)
int pos;
// time in milliseconds in which the servo is turned (for chickens: 8 hours = 8h * 60m * 60s * 1000ms = 28800000
const long servoFrequency = 28800000; //60000;

// variable to know when to tilt!
long startTime = 0;
long elapsedTime = 0;

// The lcd control
// the value of the potentiometer which stears lcd
int potVal;
// display rotator
int lcdSwitch = 0;

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
  //// SET THE SERVO STARTING ANGLE
  //angle = rightAngle;
  //myServo.write(angle);
  
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

  // READ CHEAP TEMP
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
  // => ((voltage - 500 mV*1V/1000mV) times 1000mV/1V/10mV)
  float temperature = (voltage - .5) * 100;

  Serial.print(" AVG[");
  Serial.print(averagetemp);
  Serial.print("]");
  Serial.print("Target[");
  Serial.print(targetTemp);
  Serial.print("] Actual cheap[");
  Serial.print(temperature);
  Serial.println("]");

  int chk = DHT.read11(DHT11_PIN);
  // delay to give temp and humidity sensors time to read
  delay(1000);
  dhtTemp = DHT.temperature;
  Serial.print("Temperature Velleman = ");
  Serial.print(dhtTemp);
  Serial.print(" Humidity = ");
  Serial.println(DHT.humidity);

  // TURN LIGHT ON / OFF TO MANIPULATE TEMPERATURE
  if(dhtTemp > 1) {
    // correct reading, proceed
    if (dhtTemp < targetTemp - 0.0) { // + 2) {
        // too cold: turn light on
        digitalWrite(LIGHT_BULB_INDEX, HIGH);
    } else if (dhtTemp > targetTemp + 0.0) {
        // too warm: turn light off
        digitalWrite(LIGHT_BULB_INDEX, LOW);
    } else {
        // keep lamp on or off, temp is OK
    }
  }

  // turn the eggs every 8 hour! = 3600s * 8
  // calculate the time sinds last check
  elapsedTime = millis() - startTime;
  if(elapsedTime >= servoFrequency) {
    startTime = millis();
    if (angle == leftAngle) {
      angle = rightAngle;
      for (pos = leftAngle; pos <= rightAngle; pos += 1) { // goes from 0 degrees to 180 degrees
        // in steps of 1 degree
        myServo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(150);                       // waits 15ms for the servo to reach the position
        Serial.print("SERVO MOVED TO POS ");
        Serial.println(myServo.read());
      }
    } else {
      angle = leftAngle;
      for (pos = rightAngle; pos >= leftAngle; pos -= 1) { // goes from 180 degrees to 0 degrees
        myServo.write(pos);              // tell servo to go to position in variable 'pos'
        delay(150);                       // waits 15ms for the servo to reach the position
        Serial.print("SERVO MOVED TO POS ");
        Serial.println(myServo.read());
      }
    };
    // wait for right pos
    delay(1000);
    Serial.print("EGGS WERE TURNED TO ANGLE ");
    Serial.print(angle);
    Serial.print(" at ");
    Serial.print((millis() / 1000));
    Serial.println("s");
  }

  // read the value of the potentio switch to switch text on display
  potVal = analogRead(potPin);
  // change the 1023 bits to 5 posibilities
  lcdSwitch = map(potVal, 0, 1023, 0, 5);
  // Display
  lcd.clear();
  switch (lcdSwitch) {
    case 0:
      // DISPLAY TARGETTEMP & SERVO ANGLE
      // DISPLAY MEASURED TEMP & HUMIDITY
      lcd.setCursor(0, 0);
      lcd.print("t");
      lcd.print(targetTemp);
      lcd.print(" angle");
      lcd.println(angle);
      lcd.setCursor(0, 1);
      lcd.print("T");
      lcd.print(DHT.temperature);
      lcd.print(" M");
      lcd.print(DHT.humidity);
      lcd.println("%");
      break;
    case 1:
      // DISPLAY COLLAPSED TIME SINDS BREADING STARTED
      lcd.setCursor(0, 0);
      lcd.print("2.Bread(h)");
      lcd.println((float)millis() / 3600000);
      lcd.setCursor(0, 1);
      lcd.print("Bread(d)");
      lcd.println((int)millis() / 3600000 / 24);
      break;
    case 2:
      // DISPLAY SERVO SETTINGS & LAST ROTATION
      lcd.setCursor(0, 0);
      lcd.print("3.ServoFr(m)");
      float freq = servoFrequency / 1000 / 60;
      lcd.println(freq);
      lcd.setCursor(0, 1);
      lcd.print("LastRot(m)");
      float rot = (millis() - startTime);
      rot = rot / 1000;
      rot = rot / 60;
      Serial.print("Last Rot millis()");
      Serial.print(millis());
      Serial.print("< starttime>");
      Serial.print(startTime);
      Serial.print("< >");
      Serial.print("< timePast>");
      Serial.println(rot);
      lcd.println(rot);
      break;
    case 3:
      // DISPLAY CHEAP TEMP SENSOR AVG READING
      lcd.setCursor(0, 0);
      lcd.print("4.ServoAngle ");
      lcd.println((int)angle);
      lcd.setCursor(0, 1);
      lcd.print("Cheap t avg");
      lcd.println(temperature);
      break;
    default:
      // DISPLAY CREDITS
      lcd.setCursor(0, 0);
      lcd.println("  KOEN MESTDAG");
      lcd.setCursor(0, 1);
      lcd.println("* * * * * * * *");
      break;
  }

  // wait for x * 1000ms so that the lamps do not keep going on and off
  delay(100);
}
