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

// target room temperature in Celsius
const float targetTemp = 37.0;
float vellemanAveragetemp = 37;
float vellemanTemps[25];
float dhtTemp;

// second temp measurement
// Fix the "floating" temperature reading
float averagetemp = targetTemp; // starting average (confirms to 37°)
float temps[25]; // use average of 25 readings because there is to much fluctuation

// Set up the LCD
// include the LCD code library:
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

// Set up the temp & moisture for the Velleman component
#include <dht.h>
dht DHT; // load class to connect to Velleman
#define DHT11_PIN 7 // set pin of temp reader

// set up servo
#include <Servo.h>
// servo object which represents the real thing
Servo myServo;
// the pin that is connected to the servo
int const potPin = A1;
// angle switches
const int leftAngle = 70;
const int rightAngle = 125;
// the current angle
int angle = 120;
// step by step rotation (local var)
int pos;
// time in milliseconds in which the servo is turned (for chickens: 8 hours = 8h * 60m * 60s * 1000ms = 28800000
const long servoFrequency = 28800000; //15000;

// variable to know when to tilt!
long startTime = 0; // time sinds startup
long elapsedTime = 0; // time sinds last turn
// pushbutton to manually control the turn
const int servoButtonPin = 8;
int buttonState = 0;         // variable for reading the pushbutton status

// variables to know how long light is on / off
boolean lampOn = false;
long lampOnTime = 0;
long lampOffTime = 0;

// Alarm piezo
int piezoPin = 10;

// The lcd control
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
  lcd.setCursor(0, 0);
  lcd.print("STARTING        ");
  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);
  lcd.print("EGG BREADING    ");
  // display & load servo capacitators
  delay(1000);

  int i;
  int chk = DHT.read11(DHT11_PIN);
  // delay to give temp and humidity sensors time to read
  delay(1000);
  dhtTemp = DHT.temperature;
  // initialize averagetemp
  for (i = 0; i < 25; i++) {
    vellemanTemps[i] = dhtTemp;
  }

  // initialize second temp reading
  averagetemp = getCheapTempReading(sensorPin);
  for (i = 0; i < 25; i++) {
    temps[i] = averagetemp;
  }

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
  
  // var to keep track of tilting
  startTime = millis();
  elapsedTime = 0;
  // initialize the pushbutton pin as an input:
  pinMode(servoButtonPin, INPUT);

  Serial.print("STARTING WITH TARGET T = ");
  Serial.println(targetTemp);
  Serial.print("STARTED at ");
  Serial.println(dhtTemp);
}

void loop() {  

  int i;
  int chk = DHT.read11(DHT11_PIN);
  // delay to give temp and humidity sensors time to read
  delay(1000);
  dhtTemp = DHT.temperature;

  if(dhtTemp > 1) { // only when good reading: calculate moving temp and stear lamps
    vellemanAveragetemp = 0;
    for (i = 24; i > 0; i--) {
      vellemanAveragetemp = vellemanAveragetemp + vellemanTemps[i];
      vellemanTemps[i] = vellemanTemps[i - 1];
    }
    vellemanTemps[0] = dhtTemp; // at last reading
    vellemanAveragetemp = vellemanAveragetemp + vellemanTemps[0];
    vellemanAveragetemp = vellemanAveragetemp / 25; // calculate average: sum all and devide by count
    
    Serial.print("Velleman temperature: ");
    Serial.print(dhtTemp);
    Serial.print(" AVG:");
    Serial.print(vellemanAveragetemp);
    Serial.print(" Humidity = ");
    Serial.println(DHT.humidity);
  
    // Turn light on / off to manipulate temperature
    // correct reading, proceed
    if (vellemanAveragetemp < (targetTemp - 0.5)) {
        // too cold: turn light on

        // lamp could be already on: check
        if(!lampOn) {
          // start clock
          lampOnTime = millis();
          lampOn = true;
        }
        
        digitalWrite(LIGHT_BULB_INDEX, HIGH);
        Serial.print("Turning lamp on at ");
        Serial.println(dhtTemp);
        delay(1000); // keep from switching on and off again
    } else if ((vellemanAveragetemp > (targetTemp + 0.9)) || (dhtTemp >= targetTemp + 2)) {
        // when average too warm: turn light off, of when to hot: immediately cool down!

        // lamp could be already off: check
        if(lampOn) {
          // start clock
          lampOffTime = millis();
          lampOn = false;
        }
        
        digitalWrite(LIGHT_BULB_INDEX, LOW);
        Serial.print("Turning lamp off at ");
        Serial.println(dhtTemp);
        delay(1000); // keep from switching on and off again
    } else {
        // keep lamp on or off, temp is OK
    }
/*  } else {
    // meter malfunction!!
    tone(piezoPin, 1000, 100); // arguments: stear piezopin, frequency 1000, ms to play) */
  }

/*  if(dhtTemp < 35) {
    tone(10, 1000);//, 500);
    Serial.println("Piezo ON");
    delay(600);
    Serial.println("Piezo OFF");
  }
*/

  // second temp reading temperature varies to much: calculate average!
  float temperature = getCheapTempReading(sensorPin);

  averagetemp = 0;
  for (i = 24; i > 0; i--) {
    averagetemp = averagetemp + temps[i];
    temps[i] = temps[i - 1];
  }
  temps[0] = temperature;
  averagetemp = averagetemp + temps[0];
  averagetemp = averagetemp / 25;

  Serial.print("Cheap: AVG[");
  Serial.print(averagetemp);
  Serial.print("]");
  Serial.print("Target[");
  Serial.print(targetTemp);
  Serial.print("] Actual [");
  Serial.print(temperature);
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
  
  if((elapsedTime >= servoFrequency) || (buttonState == HIGH)) {
    startTime = millis();
    if (angle == leftAngle) {
      angle = turnUp(leftAngle, rightAngle, 150);
    } else {
      angle = turnDown(rightAngle, leftAngle, 150);
    };
    
    // i am having a problem with scrambled chars on my lcd screen: so reset every 8 hours :)
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.println("RESET LCD       ");
    lcd.setCursor(0, 1);
    lcd.println("RESET LCD       ");
    
    // wait for right pos
    delay(1000);
    Serial.print("Eggs were turned to angle ");
    Serial.print(angle);
    Serial.print(" at ");
    Serial.print((millis() / 1000));
    Serial.println("s");
  }

  // read the value of the potentio switch to switch text on display
  potVal = analogRead(potPin);
  
  // change the 1023 bits to 6 posibilities
  lcdSwitch = map(potVal, 0, 1023, 0, 6);
  Serial.print("Potentiometer: ");
  Serial.print(potVal);
  Serial.print(" lcdSwitch ");
  Serial.println(lcdSwitch);
  
  // Display
  //  lcd.clear(); // trying to eliminate weard chars
  String lcdText;
  if (lcdSwitch == 0) {
      // DISPLAY TARGETTEMP & SERVO ANGLE
      // DISPLAY MEASURED TEMP & HUMIDITY
      lcd.setCursor(0, 0);
      lcdText = appendSpaces(String("T" + String(DHT.temperature) + " M" + String(DHT.humidity)), 16);
      lcd.println(lcdText);
      lcd.setCursor(0, 1);
      lcdText = appendSpaces(String("Avg temp" + String(vellemanAveragetemp, 1)), 16);
      lcd.println(lcdText);
      Serial.println("case 0");
 } else if(lcdSwitch == 1) {
      // DISPLAY COLLAPSED TIME SINDS BREADING STARTED
      lcd.setCursor(0, 0);
      float timeCollapsed = (millis() - lampOffTime) / 1000;
      lcdText = appendSpaces(String("LampOff s" + String(timeCollapsed, 1)), 16);
      Serial.print(lcdText + "  > ");
      Serial.print(String(lampOffTime, 1));
      Serial.print(" > ");
      Serial.println(String(millis(), 1));
      lcd.println(lcdText);
      // DISPLAY TIME LAMP ON /OFF
      lcd.setCursor(0, 1);
      timeCollapsed = (millis() - lampOnTime) / 1000;
      lcdText = appendSpaces(String("LampOn s" + String(timeCollapsed, 1)), 16);
      Serial.print(lcdText);
      Serial.print(" >> ");
      Serial.print(String(lampOnTime, 1));
      Serial.print(" >> ");
      Serial.println(String(millis(), 1));
      Serial.println("case 1");
      lcd.println(lcdText);
 } else if(lcdSwitch == 2) {
      // DISPLAY SERVO SETTINGS & LAST ROTATION
      lcd.setCursor(0, 0);
      float freq = servoFrequency / 1000 / 60;
      lcdText = appendSpaces(String("3.ServF(m)" + String(freq, 1)), 16);
      lcd.println(lcdText);
      lcd.setCursor(0, 1);
      float rot = (millis() - startTime);
      rot = rot / 1000 / 60;
      lcdText = appendSpaces(String("ServTurn(m)" + String(rot, 2)), 16);
      lcd.println(lcdText);
      Serial.println("case 2");
 } else if(lcdSwitch == 3) {
      // DISPLAY TEMP SENSOR AVG READING
      Serial.println("case 3");
      lcd.setCursor(0, 0);
      lcdText = appendSpaces(String("Servo " + String(angle)), 16);
      lcd.println(lcdText);
      lcd.setCursor(0, 1);
      lcdText = appendSpaces(String("t" + String(targetTemp, 1) + " angle " + angle), 16);
      lcd.println(lcdText);
      Serial.println("case 3");
  } else if(lcdSwitch > 3){
      // DISPLAY CREDITS
      lcd.setCursor(0, 0);
      lcd.println("  KOEN MESTDAG  ");
      lcd.setCursor(0, 1);
      lcd.println("* * * * * * * * ");
      Serial.println("case default");
  }

  // wait for x * 1000ms so that the lamps do not keep going on and off
  delay(100);
}

/* Function to rotate servo from low to high position */
/* low: startpos, high: endpos, delayTime: the time the servo pauze to proceed to next degree */
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

/* Function to rotate servo from high to low position */
int turnDown(int high, int low, int delayTime) {
  int pos;
  for (pos = high; pos >= low; pos -= 1) { // goes from 180 degrees to 0 degrees
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(150);                       // waits 15ms for the servo to reach the position, to not throw the eggs to the wall
    Serial.print("SERVO MOVED TO POS ");
    Serial.println(myServo.read());
  }
  return (myServo.read());
}

/* Function which reads temp of sensor */
float getCheapTempReading(int pin) {
  // read the value on AnalogIn pin 0 (temp sensor) and store it in a variable
  int sensorVal = analogRead(pin);
  // convert the ADC reading to voltage
  float voltage = (sensorVal / 1024.0) * 5.0;
  // convert the voltage to temperature in degrees C
  // the sensor changes 10 mV per degree
  // the datasheet says there's a 500 mV offset
  // => ((voltage - 500 mV*1V/1000mV) times 1000mV/1V/10mV)
  float temperature = (voltage - .5) * 100;
  return temperature;
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

