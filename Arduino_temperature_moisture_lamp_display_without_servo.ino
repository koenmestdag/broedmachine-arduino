/*
  Arduino egg hatcher

  Parts required:
  - one TMP36 temperature sensor            // double check on breading temp
  - 1 red LEDs                              // shows when heating is on
  - 2 220 ohm resistor                      // protects LED, helps pushbutton
  - Velleman VMA311 (DHT11)                 // used for regulating breading temp & humidity
  - Relco VLM 99085 snoerdimmer             // used for diming IR light
  - Velleman 4 channel relay module VMA400  // IR light switch
  - 1 servo motor                           // servo motor to tilt eggs
  - 2 XXX capacitors                        // protect board from servo creating power dip
  - 1 1J63 capacitor (0.1µF = 100nF)        // used to shield lcd from servo peak

  created 9 februari 2019
  by Koen Mestdag

  This example code is part of the public domain.
*/

// named constant for the pin the temp sensor is connected to
const int sensorPin = A0;
// lightbulbindex
const int LIGHT_BULB_INDEX = 2;

// *** Temperature reading ***
// target room temperature in Celsius
const float targetTemp = 37.0;

// main temperature (Velleman temp reader VMA311)
float averageTemperature1; // instantiate average
float temperature1Readings[25];
float temperature1;
float temperature1Correction = -1.0;   // manually adjust digital meter

// second temp measurement
// Fix the "floating" temperature reading
float averageTemperature2; // instantiate average
float temperature2Readings[25]; // use average of 25 readings because there is to much fluctuation
float temperature2;
float temperature2Correction = 2.0;   // manually adjust digital meter

// Set up the LCD
// include the LCD code library:
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

// Set up the temp & moisture for the Velleman component
#include <dht.h>
dht DHT; // load class to connect to Velleman
#define DHT11_PIN 7 // set pin of temperature reader

/*
// set up servo
#include <Servo.h>
// servo object which represents the real thing
Servo myServo;
// angle switches
const int leftAngle = 70;
const int rightAngle = 125;
// the current angle
int angle = 120;
// step by step rotation (local var)
int pos;
// time in milliseconds in which the servo is turned (for chickens: 8 hours = 8h * 60m * 60s * 1000ms = 28800000
const long servoFrequency = 28800000; //15000;
*/

// variable to know when to tilt!
long startTime; // last turn time
long elapsedTime = 0; // time passed sinds last turn
// pushbutton to manually control the turn
const int servoButtonPin = 8;
int buttonState = 0;         // variable for reading the pushbutton status

// variables to know how long light is on / off
boolean lampOn = false;
long lampOnTime = 0;
long lampOffTime = 0;

// Alarm piezo
int piezoPin = 9;

// The lcd control
// the pin that is connected to the potentiometer
int const potPin = A1;
// the value of the potentiometer which stears lcd
int potVal;
// display rotator
int lcdSwitch = 0;

void setup() {
  // open a serial connection to display values on computer
  Serial.begin(9600);
  
  // set the LED / IR-lamp pin as outputs
  pinMode(LIGHT_BULB_INDEX, OUTPUT);
  // turn LAMP ON
  lampOn = true;
  digitalWrite(LIGHT_BULB_INDEX, HIGH);

  // start up the display
  // set up the number of columns and rows on the LCD
  lcd.begin(16, 2);
  // Print a message to the LCD.
  displayMessage("EGG BREADING", "MESTDAG");
  for(int i = 0; i < 30; ++i) {
    tone(piezoPin, (i * 100), 100);
    // display delay & make some music :-0
    delay(100);
  }

  int i;
  temperature1 = getTemperature1Reading(DHT11_PIN);
  // initialize averageTemperature2
  for (i = 0; i < 25; i++) {
    temperature1Readings[i] = temperature1;
  }
  averageTemperature1 = temperature1;

  // initialize second temp reading
  temperature2 = getTemperature2Reading(sensorPin);
  for (i = 0; i < 25; i++) {
    temperature2Readings[i] = temperature2;
  }
  averageTemperature2 = temperature2;

/*
  // setup servo: attach servo object to Arduino digital out 9
  myServo.attach(9);
  angle = myServo.read();
  delay(500);
  // SET THE SERVO STARTING ANGLE > otherwise the servo goes very fast to rightangle pos
  if(angle < 90) {
    Serial.print("Moving servo from ");
    Serial.print(angle);
    Serial.println(" to 180");
    angle = turnUp(angle, rightAngle, 50);
  } else {
    Serial.print("Moving servo from ");
    Serial.print(angle);
    Serial.println(" to 60");
    angle = turnDown(angle, leftAngle, 50);
  }
*/
  
  // var to keep track of tilting
  startTime = millis() - 300000; // make sure alarm can start immediately
  elapsedTime = 0;
  // initialize the pushbutton pin as an input:
  pinMode(servoButtonPin, INPUT);

  Serial.print("STARTING WITH TARGET T = ");
  Serial.println(targetTemp);
  Serial.print("STARTED at ");
  Serial.println(temperature1);
}

void loop() {  

  int i;
  temperature1 = getTemperature1Reading(DHT11_PIN);    // read current temp
  if(temperature1 > 1) { // only when good reading: calculate moving temp and stear lamps
    averageTemperature1 = 0;
    for (i = 24; i > 0; i--) {
      averageTemperature1 = averageTemperature1 + temperature1Readings[i];
      temperature1Readings[i] = temperature1Readings[i - 1];
    }
    temperature1Readings[0] = temperature1; // at current reading
    averageTemperature1 = averageTemperature1 + temperature1Readings[0];
    averageTemperature1 = averageTemperature1 / 25; // calculate average: sum all and devide by number of readings
    Serial.println(String("Velleman temperature:" + String(temperature1, 1) + " AVG:" + String(averageTemperature1, 1) + " Humidity = " + String(DHT.humidity, 1)));
  
    // Turn light on / off to manipulate temperature
    // correct reading, proceed
    if (averageTemperature1 < (targetTemp - 0.5)) {
        // too cold: turn light on
        if(!lampOn) {        // lamp could be already on: check
          // start clock
          lampOnTime = millis();
          lampOn = true;
          digitalWrite(LIGHT_BULB_INDEX, HIGH);
          Serial.println(String("Turning lamp on at " + String(temperature1, 1)));
        }
    } else if ((averageTemperature1 > (targetTemp + 0.9)) || (temperature1 >= targetTemp + 2)) {
        // when average too warm: turn light off, of when to hot: immediately cool down!
        if(lampOn) {           // lamp could be already off: check
          // start clock
          lampOffTime = millis();
          lampOn = false;
          digitalWrite(LIGHT_BULB_INDEX, LOW);
          Serial.print("Turning lamp off at ");
          Serial.println(temperature1);
        }
    } else {
        // keep lamp on or off, temp is OK
    }
  } else {
    // meter malfunction!!
    tone(piezoPin, 1000, 900);        // arguments: piezopin, frequency 1000, ms to play)
    displayMessage("ALARM SENSOR!", String("T" + String(temperature1, 2)));
    Serial.println("ALARM: SENSOR MALFUNCTION!!");
    delay(1000);
  }

  long startPlus5min = (startTime + 300000);
  Serial.println(String("Starttime" + String(startTime) + " starttime " + String(startPlus5min)  + "  millis " +  String(millis())));

  /* Sound alarm when temperature drops! */
  if(startPlus5min < millis()) {      // if alarm not pauzed for 5' and has been minimum 5 mins sinds startup
    if(averageTemperature1 <= (targetTemp - 2)) {
      tone(piezoPin, 1000, 900);
      displayMessage(String("ALARM temp " + String(temperature1, 1)), String("Average T " + String(averageTemperature1, 2)));
      Serial.println("ALARM: Temperature to low!!");
      delay(2000);
    } else if ((averageTemperature1 - averageTemperature2) > 40) {
      tone(piezoPin, 800, 250);
      displayMessage("ALARM DIFF!!", String("t1:" + String(averageTemperature1, 1) + " t2:" + String(averageTemperature2, 2)));
      Serial.println("ALARM: Temperature to low!!");
      delay(2000);
    }
  }

  // second temp reading temperature varies to much: calculate average!
  temperature2 = getTemperature2Reading(sensorPin);    // read current temp
  averageTemperature2 = 0;
  for (i = 24; i > 0; i--) {
    averageTemperature2 = averageTemperature2 + temperature2Readings[i];
    temperature2Readings[i] = temperature2Readings[i - 1];
  }
  temperature2Readings[0] = temperature2;
  averageTemperature2 = averageTemperature2 + temperature2Readings[0];
  averageTemperature2 = averageTemperature2 / 25;

  Serial.print("T2: AVG[");
  Serial.print(averageTemperature2);
  Serial.print("]");
  Serial.print("Target[");
  Serial.print(targetTemp);
  Serial.print("] Actual [");
  Serial.print(temperature2);
  Serial.println("]");

  // Turn the eggs when manual button is pressed. If it is, the buttonState is HIGH.
  // OR turn the eggs every servoFrequency seconds (should be 8 hours)
  // -> read the state of the pushbutton value:
  buttonState = digitalRead(servoButtonPin);
  if(buttonState == HIGH) {
    Serial.println("BUTTON PRESSED!!");
  }
  // Calculate the time sinds last check
  elapsedTime = millis() - startTime;
  
//  if((elapsedTime >= servoFrequency) || (buttonState == HIGH)) {
  if(buttonState == HIGH) {
    startTime = millis();  // reset turn time

/*    
    if (angle == leftAngle) {
      angle = turnUp(leftAngle, rightAngle, 150);
    } else {
      angle = turnDown(rightAngle, leftAngle, 150);
    };
    
    // wait for right pos
    delay(1000);
    Serial.print("Eggs were turned to angle ");
    Serial.print(angle);
    Serial.print(" at ");
    Serial.print((millis() / 1000));
    Serial.println("s");
*/    
    // i am having a problem with scrambled chars on my lcd screen: so reset every 8 hours :)
    lcd.begin(16, 2);
    displayMessage("BUTTON PUSHED", "RESET LCD");
    delay(500);
  }

  // read the value of the potentio switch to switch text on display
  potVal = analogRead(potPin);
  
  // change the 1023 bits to 6 posibilities
  lcdSwitch = map(potVal, 0, 1023, 0, 6);
  Serial.print("Potentiometer: ");
  Serial.print(potVal);
  Serial.print(" lcdSwitch ");
  Serial.println(lcdSwitch);
  
  /* Display */
  String lcdText1;
  String lcdText2;
  if (lcdSwitch == 0) {
      // DISPLAY TARGETTEMP & SERVO ANGLE
      // DISPLAY MEASURED TEMP & HUMIDITY
      lcdText1 = String("T" + String(temperature1) + " M" + String(DHT.humidity));
      lcdText2 = String("Avg temp" + String(averageTemperature1, 1));
      Serial.println("case 0");
 } else if(lcdSwitch == 1) {
      // Display last / current lampoff time
      float varOff;
      varOff = getLampOffTime();
      lcdText1 = String("LampOff s" + String(varOff, 1));  // getLampOffTime()
      // Display last / current lampon time
      float varOn = 0;
      varOn = getLampOnTime();
      lcdText2 = String("LampOn s" + String(varOn, 1));   // getLampOnTime()
      Serial.println("case 1");
 } else if(lcdSwitch == 2) {
      // DISPLAY SERVO SETTINGS & LAST ROTATION
/*      float freq = servoFrequency / 1000 / 60;
      lcdText1 = appendSpaces(String("3.ServF(m)" + String(freq, 1)), 16);
      float rot = (millis() - startTime);
      rot = rot / 1000 / 60;
      lcdText2 = appendSpaces(String("ServTurn(m)" + String(rot, 2)), 16);*/
      // DISPLAY TEMP SENSORS
      lcdText1 = String("t1:" + String(temperature1, 1) + " t2:" + String(temperature2, 1));
      lcdText2 = String("DHT:" + String(DHT.temperature, 1) + " act:");
      Serial.println("case 2");
 } else if(lcdSwitch == 3) {
      // DISPLAY AVG TEMP SENSORS
      lcdText1 = String("Average");
      lcdText2 = String("t1:" + String(averageTemperature1, 1) + " t2:" + String(averageTemperature2, 1));
      Serial.println("case 3");
  } else {
      // DISPLAY CREDITS
      lcdText1 = "* KOEN MESTDAG *";
      lcdText2 = "* * * *  * * * *";
      Serial.println("case default");
  }
  displayMessage(lcdText1, lcdText2);

  // wait for x * 1000ms so that the lamps do not keep going on and off
  delay(100);
}

/* Function to rotate servo from low to high position */
/* @param low: startpos, @param high: endpos, @param delayTime: the time the servo pauze to proceed to next degree */
/*
int turnUp(int low, int high, int delayTime) {
  int pos;
  for (pos = low; pos <= high; pos += 1) { // goes from LOW degrees to HIGH degrees (min 0- max 180)
    // in steps of 1 degree
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(150);                       // waits 15ms for the servo to reach the position, to not throw the eggs to the wall
    Serial.print("SERVO MOVED TO POS ");
    Serial.println(myServo.read());
  }
  return (myServo.read());
}
*/

/* Function to rotate servo from high to low position */
/*
int turnDown(int high, int low, int delayTime) {
  int pos;
  for (pos = high; pos >= low; pos -= 1) { // goes from 180 degrees to 0 degrees
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(150);                       // waits 15ms for the servo to reach the position, to not throw the eggs to the wall
    Serial.print("Servo moved to pos ");
    Serial.println(myServo.read());
  }
  return (myServo.read());
}
*/

/* Temperature sensor 1 reader */
float getTemperature1Reading(int pin) {
  // delay to give temp and humidity sensors time to read
  delay(1000);
  int chk = DHT.read11(DHT11_PIN); // readout Temperature1 sensor  read21 has greater accuracy then .read11(DHT11_PIN)
  return DHT.temperature + temperature1Correction;
}

/* Temperature sensor 2 reader */
float getTemperature2Reading(int pin) {
  float result;
  // read the value on AnalogIn pin 0 (temp sensor) and store it in a variable
  int sensorVal = analogRead(pin);
  // convert the ADC reading to voltage
  float voltage = (sensorVal / 1024.0) * 5.0;
  // convert the voltage to temperature in degrees C
  // the sensor changes 10 mV per degree
  // the datasheet says there's a 500 mV offset
  // => ((voltage - 500 mV*1V/1000mV) times 1000mV/1V/10mV)
//  float temperature = (voltage - .5) * 100;
  result = ((voltage - .5) * 100);
  return result + temperature2Correction;
}

/* Returns string of length len, by appending spaces to the right */
String appendSpaces(String text, int len) {
  String iText = text;
  if(iText.length() > len) {
    iText = iText.substring(1, 17);
  }
  while (iText.length() < len) {
    iText.concat(" ");
  };
  return iText;
}

/* Renders message on lcd display */
void displayMessage(String message1, String message2) {
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print(appendSpaces(message1, 16));
  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);
  lcd.print(appendSpaces(message2, 16));
}

float getLampOnTime() {
  float referenceTimeOn;
  if(lampOn) {
    referenceTimeOn = millis();
  } else {
    referenceTimeOn = lampOffTime;
  }
  float timeOnCollapsed = ((referenceTimeOn - lampOnTime) / 1000);
  return timeOnCollapsed;
}

float getLampOffTime() {
  float referenceTimeOff;
  if(lampOn) {
    referenceTimeOff = lampOnTime;  // lamp is now on:  measure the off time between last lamp off time and last lamp on
  } else {
    referenceTimeOff = millis();    // lamp is now off: measure the off time between lamp off starttime and current time
  }
  float timeOffCollapsed = (referenceTimeOff - lampOffTime) / 1000;
  return timeOffCollapsed;
}
