/*
 * Datum: 15.12.2021
 * Projekt: ein Robot Car für das Informatik 1 Labor, per Blynk App steuerbar 
 * Hardware: ESP32 Dev Mode, 4x DC Motoren, 2x L298N Schrittmotor Treibermodul, 2x 9V und 4x 1.5V Batterien
 */

/*
 * Blynk spezifische Einstellungen
 */
#define BLYNK_TEMPLATE_ID "XXXXXXXXXXXX"                      // Authentifizierungscode
#define BLYNK_DEVICE_NAME "JoyIt car"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG

/*
 * Verwendete Libraries
 */
#include "BlynkEdgent.h"                                      // BlynkApp Bibliothek
#include <Robojax_L298N_DC_motor.h>                           // Schrittmotor Treiber Bibliothek

/*
 * Fuer die beiden Motoren werden die PINs 5/18/19 und 12/14/27 verwendet.
 */
#define motor1 1                                              // Motor 1 zugehörige PINS
#define ENA 19
#define IN1 18
#define IN2 5
                              
#define motor2 2                                              // Motor 2 zugehörige PINS
#define IN3 14
#define IN4 12
#define ENB 27 

/*
 * Globale Variablen
 */
Robojax_L298N_DC_motor robot(IN1, IN2, ENA, IN3, IN4, ENB);   // Ansteuerung Motor/Motor-Treiber
uint8_t motorSpeed;
int x_direction;
int y_direction;
const int CCW = 2;                                            // Drehrichtung "Gegen den Uhrzeigersinn"
const int CW  = 1;                                            // Drehrichtung "Im Uhrzeigersinn"

/*
 * Diese Funktion liest die Eingaben des Joysticks durch die Blynkapp ein und speichert diese in unseren globalen Variablen.
 */
BLYNK_WRITE(V10) {
  int x = param[0].asInt();
  int y = param[1].asInt();

  x_direction = input_normalisieren(x);
  y_direction = input_normalisieren(y);
}

/*
 * Diese Funktion normalisiert die Eingabe durch den Joystick (WB 0-255/0-255) auf den Wertebereich -128 bis 127.
 * Verwendet fuer die X und die Y Achse.
 */
int input_normalisieren(int input) {
  return input - 128;
}

/*
 * Diese Funktion steuert die Motoren abhaengig vom Winkel in dem der Joystick bewegt wurde.
 */
void fahren(double winkel) {
  if (winkel >= 45 && winkel <= 135) {
    // vorwaerts
    robot.rotate(motor1, motorSpeed, CW);
    robot.rotate(motor2, motorSpeed, CCW);
  }
  else if (winkel > 135 && winkel < 225) {
    // links
    robot.rotate(motor1, motorSpeed, CCW);
    robot.rotate(motor2, motorSpeed, CCW);
  }
  else if (winkel >= 225 && winkel <= 315) {
    // rueckwaerts
    robot.rotate(motor1, motorSpeed, CCW);
    robot.rotate(motor2, motorSpeed, CW);
  }
  else {
    // rechts
    robot.rotate(motor1, motorSpeed, CW);
    robot.rotate(motor2, motorSpeed, CW);
  }
}

/*
 * Diese Funktion entscheidet, ob sich die Motoren bereits bewegen sollen. 
 * Ist die Laenge des Richtungsvektor echt kuerzer als 20, so darf sich kein Motor bewegen.
 */
bool darf_fahren(int x, int y) {
  return sqrt((x*x)+(y*y)) > 20;
}

/*
 * Diese Funktion berechnet aufgrund der Eingabe durch den Joystick mit dessen X- und Y-Werten den Winkel in dem der Joystick bewegt wurde.
 */
double winkel(int x, int y) {
  double winkel = 0.0; 
  double nenner =  (x * x);
  double zaehler = sqrt(pow(x, 2) + pow(y, 2)) * x;
  double acosinhalt = nenner == 0 ? 0.0 : nenner / zaehler;
  
  winkel = (acos(acosinhalt)*180.0)/M_PI;

  if (y < 0) {
    winkel = (90 - winkel) + 270.0;
  }

  return winkel;
}

/*
 * Setup Funktion des Programms.
 * 
 * Wird einmalig ausgefuehrt und setzt den Motorspeed und startet sowohl die BlynkApp-Verbindung, als auch die Motorenverbindung.
 */
void setup()
{
  Serial.begin(115200);
  delay(100);

  motorSpeed = 100;

  BlynkEdgent.begin();
  robot.begin();
}

/*
 * Loop Funktion des Programms.
 * 
 * Wird einmal pro Loop ausgefuehrt und interpretiert die Eingabe durch den Joystick aus der BlynkApp. Duerfen sich die Motoren bewegen, so werden diese mit Hilfe des Richtungsvektor des Joysticks angesteuert.
 * Wird der Joystick nicht bewegt, so bremsen die Motoren.
 * Anschliessend folgt ein Delay, um die Loopgeschwindigkeit des Programmes zu drosseln.
 */
void loop() {  
  if(darf_fahren(x_direction, y_direction)) {
    double w = winkel(x_direction, y_direction);
    fahren(w);
  } else {
    robot.brake(1);
    robot.brake(2);  
  }
  
  BlynkEdgent.run();
  
  delay(100);
}
