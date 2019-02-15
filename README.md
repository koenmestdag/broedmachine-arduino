# broedmachine-arduino
Maak met je Arduino een broedmachine voor (kippe)eieren

Built with the Arduino Starter kit!
  + Velleman VMA311
  + Velleman VMA400
  + 220V cable
  + 40W light bulb
  + (long) jumper wires
  + isolated container

Display
-------
Connect
  + Display 16 > GND
  + Display 15 > resistor 220 Ohm > +5V
  + Display pin 14, 13, 12, 11 > Arduino Digital 3, 4, 5, 6
  + Display 6  > Arduino pin 11
  + Display 5  > GND
  + Display 4  > Arduino pin 12
  + Display 3  > Potentiometer
  + Display 2  > +5V
  + Display 1  > GND

Servo motor
-----------
Connect
  + Motor Black wire > +5V (breadboard 18)
  + Motor Red wire   > GND (breadboard 17)
  + Motor White wire > Arduino Digital 9 (breadboard 16)

Velleman VMA311 (DHT11) Temp & Humidity (breadboard 28, 29, 30)
---------------------------------------
Connect
  + S > Digital 7 (breadboard 28)
  + /+ > +5V       (breadboard 29)
  + /- > GND       (breadboard 30)

Velleman 4 channel relay module VMA400
---------------------------------------
Soldeer drie draden aan de relais op VCC, GND en IN1, verbind respectievelijk met VCC (+), GND (-) en digitale poort 2 van de Arduino (breaboard 25).

Knip 1 draad van de 220V voeding naar de lamp door en bevestig de uiteinden aan de normal closed poorten van relais 1. Opgelet: de relais is onderaan niet geïsoleerd!

1)   remove the jumper
2)   connect the external supply (9V) with + to VCC on the module and - to GND on the module.  The coils are then driven by the external.
3)   Arduino  should be connected with its GND to COM on the module and with signal wire (HIGH/LOW) to the  In-pins on the module.
