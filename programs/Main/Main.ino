/** importiert Arduino automatisch, muss man also hier nicht unbedingt auch noch mal importieren: */
#include <Arduino.h>

/**
 * !!! Immer darauf achten, dass unten in der Statusleiste...
 *     ... das richtige Arduino Board eingestellt ist
 *     ... das richtige "Sketch File" ausgewählt ist (das ändert sich nämlich nicht automatisch)
 *     ... die richtige C/C++ Konfiguration eingestellt ist (sonst gibt es noch mehr "rede swiggels")
 *
 * :: Externen RGB Farbsensor auslesen ::
 * :: Serial Plotter ausprobieren ::
 * :: Hauptprogramm-Schleife in Zustände unterteilen ::
 * :: Programm in Funktionen unterteilen ::
 *
 * Hardware-Aufbau:
 * TCS34725:    Arduino Due / Arduino Nano RP2040 Connect:
 *     [LED <-- (kann man optional an irgendeinen Digital-Pin oder den INT Pin vom Sensor anschließen,
 *              um die LED abzuschalten)]
 *     [INT  --> (kann man optional an irgendeinen Digital-Pin anschließen,
 *                wenn man den Sensor effizienter auslesen will)]
 *      SDA <-> SDA
 *      SCL <-- SDA
 *     [3V3 --> (2,3V Spannungsversorgung, falls man sowas irgendwo benötigt)]
 *      GND <-- GND
 *      VIN <-- 3V3
 * Der QTR-6RC ist ein externer Sensor, der an jeden Digital Pin angeschlossen werden kann (Siehe SENSOR_LEISTE_PINS).
 * !!! Beim Arduino Nano RP2040 Connect darf der Sensor nicht an Pins A6 oder A7 angeschlossen werden,
 *     da diese nur Eingänge sind und die Bibliothek den Pin sowohl als Ein- als auch als Ausgang verwendet.
 *----------------------------------------------------------------------------------------------------------------------
 * Der VL53L0X ist ein externer Sensor, kann also entweder
 * - an   Bus I2C0 ("Wire")
 * - oder Bus I2C1 ("Wire1") verbunden werden.
------------------------------------------------------------------------------------------------------------------------
 * Hardware-Aufbau:
 * Arduino Nano RP2040 Connect (!! Arduino Due geht nicht !!)
 * <nichts weiter>
 * Der LSM6DSOX ist immer an Bus I2C0 ("Wire") verbunden. Das kann nicht geändert werden.
 *
 * Der LSM6DSOX Sensor hängt am I2C Bus (SDA/SCL) mit 7-Bit Adresse 0x6A.
 * Die Adresse kann nicht verändert werden. Man kann also keinen anderen Sensor mit derselben Adresse
 * an den selben Bus anschließen.
*/

#include "includes.h"     // all libraries
#include "variables.h"    // all declarations and variables
#include "Calibration.h" // calibration values for reflection sensors, color sensors and potentially compass sensor

void setup()
{
  delay(5000);                       // Wichtig für den Abstandssensor
  pinMode(LED_BUILTIN, OUTPUT);      // Pin D13
  pinMode(motorPin, INPUT_PULLDOWN); // define pinmode for switch on the side of the bot
  pinMode(calibrationPin, INPUT);      // define pinmode for calibration button

  // Set the color LEDS as outputs
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  // Turn of any "lingering" LEDs
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);

  Serial.begin(115200);
  // I2C Bus 1x für alle Bus-Teilnehmer initialisieren (sonst crasht das Betriebssystem)
  Wire.begin();           // Bus I2C0
  Wire.setClock(1000000); // 1MHz Kommunikationsgeschwindigkeit
  Wire1.begin();          // Bus I2C1

  // REIHENFOLGE:
  /*
   - Abstandssensor
   - Farbsensoren
  */
  
  // ABSTANDSSENSOR-INITIALISIEREN
  Serial.println("Initialisierung des 1-Kanal ToF kann bis zu 10 Sekunden dauern...");
  tofSensor.setBus(&Wire);
  tofSensor.setAddress(NEW_TOF_ADDRESS);
  if (!tofSensor.init()) {
      delay(5000); // damit wir Zeit haben den Serial Monitor zu öffnen nach dem Upload
      Serial.println("ToF Verdrahtung prüfen! Roboter aus- und einschalten! Programm Ende.");
      while (1);
  }
  // Einstellung: Fehler, wenn der Sensor länger als 500ms lang nicht reagiert
  tofSensor.setTimeout(500);
  // Reichweiter vergrößern (macht den Sensor ungenauer)
  tofSensor.setSignalRateLimit(0.1);
  tofSensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  tofSensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  // lasse Sensor die ganze Zeit an
  tofSensor.startContinuous();

  Serial.println("Initialisierung Abstandssensor abgeschlossen");


  
  if (!rgbSensor.begin(TCS34725_ADDRESS, &Wire))
  {
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    delay(10000); // damit wir Zeit haben den Serial Monitor zu öffnen nach dem Upload
    Serial.println("RGB 1 (rechts) Farbsensor Verdrahtung prüfen!");
    while (!rgbSensor.begin(TCS34725_ADDRESS, &Wire));
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, LOW);
  }
  Serial.println("Initialisierung Farbe 1 abgeschlossen");
  if (!rgbSensor2.begin(TCS34725_ADDRESS, &Wire1)) // test colorsensor 2
  {
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDB, HIGH);
    delay(10000); // damit wir Zeit haben den Serial Monitor zu öffnen nach dem Upload
    Serial.println("RGB 2 (links) Farbsensor Verdrahtung prüfen!");    
    while (!rgbSensor2.begin(TCS34725_ADDRESS, &Wire1));
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDB, LOW);
  }
  Serial.println("Initialisierung Farbe 2 abgeschlossen");
  reflectanceSensor.setTypeRC();
  reflectanceSensor.setSensorPins(SENSOR_BAR_PINS, SENSOR_BAR_NUM_SENSORS);
  Serial.println("Initialisierung Reflektionssensor abgeschlossen");

  motors.initialize();
  // falls man global die Motor-Drehrichtung ändern möchte:
  motors.flipLeftMotor(false); // nur notwendig, wenn man true reinschreibt
  motors.flipRightMotor(true); // nur notwendig, wenn man true reinschreibt


  debug = LOG_LINE;
}

#include "Reflectance.h" // commands for reading and processing reflectionsensor
#include "Compass.h" // commands for a compass
#include "MotorMovements.h"    //predefined motor movements
#include "Colour.h"        //commands for reading and processing colorsensors
#include "Kreuzung.h"      //command for handling crosssections
#include "Opfer.h"              //Du Opfer

#include "Distance.h"            // Abstand, noch nicht einsortiert zwischen die restlichen includes

void loop()
{
  if (no_line_cycle_count >= 35)
  {
    Serial.println("opfer");
    opfer();
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);
    no_line_cycle_count = 0;
    straight();
    delay(3000);
  }

  // Serial.print(digitalRead(calibrationPin));Serial.print("\t");Serial.print(digitalRead(motorPin));;Serial.print("\n");

  if (digitalRead(calibrationPin))
  {
    delay(500);
    if (digitalRead(calibrationPin)) {
      stop();
      for (int i = 0; i < 5; i++)
      { // 5x blinken (AN/AUS):
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
      }
      // Calibrating should word by calculating an average from multiple values
      average_r = average_g = average_b = average_c = average_r2 = average_g2 = average_b2 = average_c2 = 0;
      int total_cycles = 10;
      for (int i = 0; i < total_cycles; i++) 
      {
        readColor();
        readColor2();

        average_r += red;
        average_g += green;
        average_b += blue;
        average_c += brightness;

        average_r2 += red2;
        average_g2 += green2;
        average_b2 += blue2;
        average_c2 += brightness2;
      }
      // calculate average values for both sensors
      average_r /= total_cycles;
      average_g /= total_cycles;
      average_b /= total_cycles;
      average_c /= total_cycles;
      average_r2 /= total_cycles;
      average_g2 /= total_cycles;
      average_b2 /= total_cycles;
      average_c2 /= total_cycles;
      
      // idea: calculate the ratio instead!
      blueGreenThreshold = average_g - average_b - 200;
      blueGreenThreshold2 = average_g2 - average_b2 - 200;
      redGreenThreshold = average_g - average_r - 200;
      redGreenThreshold2 = average_g2 - average_r2 - 200;

      colorBrightMaxThreshold = max(brightness, brightness2) + 1500;
      colorBrightMinThreshold = min(brightness, brightness2) - 300;

      // 738 886 767 2399

      Serial.println("Values: " + String(average_r) + " " + String(average_g) + " " + String(average_b)+ " " + String(average_r2) + " " + String(average_g2) + " " + String(average_b2) + " " + String(brightness)+ " " + String(brightness2));
      Serial.println("Thresholds: " + String(blueGreenThreshold) + " " + String(redGreenThreshold) + " " + String(blueGreenThreshold2) + " " + String(redGreenThreshold2) + " " + String(colorBrightMaxThreshold)+ " " + String(colorBrightMinThreshold));

      // Serial.println("red vals: " + String(red) + " " + String(green) + " " + String(blue) + " " + String(brightness) + "\t " + String(red2) + " " + String(green2) + " " + String(blue2) + " " + String(brightness2));
      Serial.println(String(isGreen()) + " " + String(isGreen2()));
      // 5x blinken (AN/AUS):
      for (int i = 0; i < 5; i++)
      {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(250);
        digitalWrite(LED_BUILTIN, LOW);
        delay(250);
      }
    }
  }

   if (digitalRead(motorPin)) {
    delay(100);
    
    // Reset LEDs
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, LOW);

    // Set distance array to invalid value
    for (int i = 0; i < 5; i++) distance_array[i] = 65535;

    // Reset colour:
    for (int i = 0; i < 4; i++) old_colour[i] = 0;
    for (int i = 0; i < 4; i++) old_colour2[i] = 0;


    // Debugging
    switch (debug) {
      case DONT_LOG:
        delay(10);
        break;

      case LOG_DISTANCE: 
        readDistance();
        logDistance();
        break;
        
      case LOG_COLOUR: 
        readColor();
        readColor2();
        Serial.println("red vals: " + String(red) + " " + String(green) + " " + String(blue) + " " + String(brightness) + "\t " + String(red2) + " " + String(green2) + " " + String(blue2) + " " + String(brightness2)  + "\t" + String(colorBrightMaxThreshold + 800));
        for (int i = 0; i < 4; i++) Serial.print(String(old_colour[i]) + " ");
        Serial.print("\t");
        for (int i = 0; i < 4; i++) Serial.print(String(old_colour2[i]) + " ");
        Serial.print("\t");
        Serial.print(String(isRed() && isRed2()) + " " + String(brightness <= colorBrightMaxThreshold + 800 || brightness2 <= colorBrightMaxThreshold + 800) + " ");
        if ((isRed() && isRed2()) && (brightness <= colorBrightMaxThreshold + 800 || brightness2 <= colorBrightMaxThreshold + 800)) Serial.print("REEEEEEEEED");
        Serial.println();
        break;

      case LOG_REFLECTANCE: 
        logReflection();
        break;

      case LOG_LINE: 
        Serial.println("Seen line: " + calculateReflection());
        break;
    }
  }

  else {
    redHandling();

    // if (last_side == LEFT_SIDE) {
    //   digitalWrite(LEDR, HIGH);
    //   digitalWrite(LEDG, HIGH);
    //   digitalWrite(LEDB, LOW);
    // }
    // if (last_side == RIGHT_SIDE) {
    //   digitalWrite(LEDR, HIGH);
    //   digitalWrite(LEDG, LOW);
    //   digitalWrite(LEDB, HIGH);
    // }
    // if (last_side == MIDDLE) {
    //   digitalWrite(LEDR, LOW);
    //   digitalWrite(LEDG, LOW);
    //   digitalWrite(LEDB, LOW);
    // }

    readDistance();
    if (distance_val <= obstacle_threshold) {
      abstand_umfahren();
    }

    calculatedReflection = calculateReflection(); // read the reflectionsensor and save the result in a variable to avoid changing values while processing
    Serial.println(calculatedReflection); 
    if (calculatedReflection != "noLine") no_line_cycle_count = 0;

    if (calculatedReflection == "frontalLine") { // detected crosssection 
      if (last_side == LEFT_SIDE) {
        kreuzung(true, -1);
      }
      if (last_side == RIGHT_SIDE) {
        kreuzung(true, 1);
      }
      else kreuzung(true, 0);
      last_side = MIDDLE;
    }

    else if (calculatedReflection == "sideLeftLine") {
      last_side = LEFT_SIDE;
      left_to_line();
    }

    else if (calculatedReflection == "sideRightLine") {
      last_side = RIGHT_SIDE;
      right_to_line();
    }

    else if (calculatedReflection == "normalLine") { // detected normal line
      last_side = MIDDLE;
      straight(1.8);
    }

    else if (calculatedReflection == "leftLine") { // detected a slight left line
      last_side = LEFT_SIDE;
      left_to_line();
    }

    else if (calculatedReflection == "rightLine") { // detected a slight right line
      last_side = RIGHT_SIDE;
      right_to_line();
    }

    else if (calculatedReflection == "hardleftLine") { // detected a hard left line
      last_side = LEFT_SIDE;
      left_to_line();
    }

    else if (calculatedReflection == "hardrightLine") { // detected a hard right line
      last_side = RIGHT_SIDE;
      right_to_line();
    }

    else if (calculatedReflection == "noLine") {// no line detected
      last_side = MIDDLE;
      // Serial.print("\n");
      // Serial.print("keine Linie...");
      straight(1.8);
      no_line_cycle_count++;
    }

    delay(10); // don't max out processor
  }
}
//gyatt