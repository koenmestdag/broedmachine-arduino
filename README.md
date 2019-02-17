# broedmachine-arduino
Maak met je Arduino een broedmachine voor (kippe)eieren

Built with the Arduino Starter kit! (servo, pushbutton, lcd, LED, breadboard, arduino ;-) )
  + Velleman VMA311
  + Velleman VMA400
  + 220V cable
  + 40W light bulb
  + (long) jumper wires
  + bigger breadboard
  + external power 5V
  + thermic isolated container (built from old home isolation plates)
  + 10 eggs

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
Standaard wordt de voeding van de Arduino gebruikt om de relais te sturen, dit kan de Arduino beschadigen. Voor gebruik een externe voeding:
1. Verwijder de jumper op de relais
2. Connecteer de externe voeding (5-9V) met de + aan de VCC van de relais module en met de - aan de GND van de relais module. De lamp wordt nu gevoed door de externe voeding.
3. Verbindt de Arduino  met de GND aan COM van de relais module en met de digitale poort 2 aan de In-pins van de relais module.

Knip 1 draad van de 220V voeding naar de lamp door en bevestig de uiteinden aan de normal closed poorten van relais 1. Opgelet: de relais is onderaan niet geïsoleerd!

Pushbutton
----------
De eieren worden alle 8 uur gedraaid. Om het draaien van de eieren manueel te starten voegen we een drukknop toe.
Connect
  + one side > resistor 220 Ohm > Arduino pin 8
  + other side to ground
