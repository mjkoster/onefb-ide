/*
 * Schwinn Eddy Current Exercise Bike replacement controller
 * 
 * Plugs into the 12 pin connector on the cable that emerges from the handlebar, 
 * in place of the factory built computer module. The cable signals are as follows:
 * 1: violet => 9V (negative) from the power adapter
 * 2: light blue => battery switch from the power adapter
 * 3: pink => 9V (positive) from the power adapter
 * 4: brown => N.O. Pedal switch
 * 5: white => N.O. Pedal switch
 * 6: gray => calibration resistance
 * 7: gold => calibration resistance
 * 8: green => position sensor potentiometer, higher eddy current end, connected to +5
 * 9: blue => position sensor potentiometer, wiper
 * 10: light yellow => position sensor potentiometer, lower eddy current end
 * 11: black => magnet position motor, negative for increasing eddy current, L293D OUT2
 * 12: red => magnet position motor, positive for increasing eddy current, L293D OUT1
 * 
 * Provides resistance control up/down buttons and a 0-100% resistance display that
 * scales the magnet position from no effect to very difficult to turn. This is a fixed 
 * calibration for this machine, but should incorporate a calibration adjustment for the 100% 
 * position that can be set by a technician, using the calibration potentiometer in the 
 * load assembly of the bike.
 * 
 * Measures speed by timing pedal crank rotations, and estimates power as the product of 
 * speed and resistance. With the eddy current load, resistance also increases with speed, 
 * so the end result is power can be approximated by:
 * resistance * speed^2 * torque multiplier
 * 
 * Ther is a trip timer under which totals are accumulated for distance, energy, average speed, 
 * and average power. Distance is accumulated pedal cranks * distance per crank. An energy 
 * estimate is kept by accumulating power over short measurement intervals during the trip. 
 * Average speed and power are straightforward distance/time and energy/time ratios.
 * 
 * A fast loop monitors the buttons and magnet position to implement position limits and 
 * provide for potential servo behavior. A slower timing loop is used to accumulate trip totals
 * and update the display
 */


#include <stdio.h>
#include <stdlib.h>
#include <Time.h>
#include <math.h>
#include <Wire.h>

#define true 1
#define false 0

#define UP_BUTTON 7 // upper button on the front of the box
#define DOWN_BUTTON 8 // lower button on the front of the box
#define MOTOR_UP_PWM 5 // to IN1 of the L293D, positive to increase eddy current effect
#define MOTOR_DOWN_PWM 6 // to IN2 of the L293D, positive to decrease eddy current effect
#define MAGNET_POSITION A0 // from the position feedback potentiomater larger value == higher eddy current effect
#define CALIBRATION A1 // from the calibration potentiomater, uses internal pullup, for a feedback calibration process
#define PEDAL_SWITCH 4 // magnetic reed switch, closes momentarily once per pedal crank revolution
#define TRIP_RESET 9 // button on the side of the box to reset the trip counting

#define UP_MOTOR_SPEED 30 // pwm on time, 100% = 256
#define DOWN_MOTOR_SPEED 40

#define DISPLAY_ADDRESS 0x2E // I2C address set by jumpers on the display PCB
#define DISPLAY_COLUMNS 20

#define METERS_PER_CRANK 4.5 // 0.5 m diameter wheel with a 3:1 sprocket ratio
#define RESISTANCE_TORQUE_MULTIPLIER 1 // represents N * s ( N*m / m*s^-1)
#define BASE_WORK 20 // added to resistance (0-100) for power calculation, rescaled to base+100

#define UPDATE_INTERVAL_MS 100 // ms between calculation and accumulator updates

int current_ms = millis();
int last_update_ms = current_ms; // start with Tlast=Tcurrent so there is an initial update interval measurement
// variables for trip duration
int trip_ms = 0; // trip interval time base, starts at 0 for each trip and increments by UPDATE_INTERVAL_MS on each update
int trip_seconds = 0; // scaled from trip_ms
int display_hours = 0; // hours to display hh:mm:ss
int display_mins = 0; // minutes to display hh:mm:ss
int display_sec = 0; // seconds to display hh:mm:ss

// displayed variables and labels, with display coordinates + formats, floats only
struct update_record {
  int x; // column number to display the value, starting at 0
  int y; // row nuimber to display the value, starting at 0
  int width; // total width in columns for the value display
  int decimal_places; // number of digits to the right of the decimal point for the value display
  float value; // the value to display
  int labelx; // column number to display the label, starting at 0
  int labely; // row nuimber to display the labet, starting at 0
  char* label; // label string to display
}
speed = {0,0, 4,1, 0, 5,0, "KPH"}, // distance per pedal stroke / time per pedal stroke
power = {0,1, 4,2, 0, 5,1, "KW"}, // resistance * speed^2 * torque scaling factor
trip_power = {10,1, 4,2, 0, 15,1, "KWAVG"}, // energy / elapsed time
trip_distance = {10,2, 4,1, 0, 15,2, "KM"}, // pedal strokes * distance per stroke
trip_speed = {0,3, 4,1, 0, 5,3, "KPHAVG"}, // distance / elapsed time
trip_energy = {12,3 ,4,2, 0, 17,3, "KWH"}; // Sum of power per interval over elapsed time

update_record updates[6] = { speed, power, trip_power, trip_distance, trip_speed, trip_energy };
int update_index = 0;

char display_buffer[DISPLAY_COLUMNS]; // for number printf and float to string, one line size in case we need to buffer a string

char time_string[8]; // buffer for constructing the trip time display

int resistance = -1; // magnet position scaled to 0-100% 

int pedal_switch_closed = false;
int pedal_ms = millis();
int last_pedal_ms = pedal_ms-100000; // start with a 100 second interval

int trip_reset_pressed = false;
int trip_active = false;

void setup_lcd() {
  Wire.begin(); 

  Wire.beginTransmission(DISPLAY_ADDRESS); 
  Wire.write(254); //Display command prefix
  Wire.write(160); //Sets the display Transmission Protocol Select to I2C
  Wire.write(0); //I2C Mode
  Wire.endTransmission(); 

  Wire.beginTransmission(DISPLAY_ADDRESS); 
  Wire.write(254); //Display command prefix
  Wire.write("X"); //Clear screen Command
  Wire.endTransmission(); 

  Wire.beginTransmission(DISPLAY_ADDRESS); 
  Wire.write(254); //Display command prefix
  Wire.write(84); //Turn off the block cursor
  Wire.endTransmission(); 
}

void display_float(int x,int y, float number, int width, int decimal_places) {
  Wire.beginTransmission(DISPLAY_ADDRESS); 
  Wire.write(254); //Display command prefix
  Wire.write(71); // move dursor
  Wire.write(x); //col
  Wire.write(y); //row
  dtostrf(number, width, decimal_places, display_buffer);
  Wire.write(display_buffer); 
  Wire.endTransmission(); 
}

void display_int(int x,int y, int number, char* format) {
  Wire.beginTransmission(DISPLAY_ADDRESS); 
  Wire.write(254); //Display command prefix
  Wire.write(71); // move dursor
  Wire.write(x); //col
  Wire.write(y); //row
  sprintf( display_buffer, format, number );
  Wire.write(display_buffer); 
  Wire.endTransmission(); 
}

void display_text(int x,int y, char* text) {
  Wire.beginTransmission(DISPLAY_ADDRESS); 
  Wire.write(254); //Display command prefix
  Wire.write(71); // move dursor
  Wire.write(x); //col
  Wire.write(y); //row
  Wire.write(text); 
  Wire.endTransmission(); 
}

void reset_trip() {
  if ( !trip_active) {
    trip_active = true;
    trip_ms = 0;
    trip_distance.value = 0; // km
    trip_energy.value = 0; // kWH
    trip_speed.value = 0;
    trip_power.value = 0;  
  }
  else {
    trip_active = false;
  }
}

void do_update() {
  display_int(10,0,resistance,"%3d");
  
  if (trip_active) {
    trip_ms += UPDATE_INTERVAL_MS;
    trip_seconds = round(trip_ms/1000);
    display_hours = trip_seconds/3600;    
    display_mins = (trip_seconds-display_hours*3600)/60;
    display_sec = trip_seconds-display_hours*3600-display_mins*60;
    String HrMinSec = (String(display_hours,2) + ":" + String(display_mins,2) + ":" + String(display_sec,2));  
    HrMinSec.toCharArray(time_string,8);
    display_text(0,2,time_string);
    trip_energy.value += power.value * (UPDATE_INTERVAL_MS/1000/3600); // kW to kWH accumulate per update period
    trip_speed.value = trip_distance.value/(trip_seconds/3600); // km / hours average for trip
    trip_power.value = trip_energy.value/(trip_seconds/3600); // kWH to kW average for trip
    }
    
  for (update_index=0; update_index < sizeof(updates)/sizeof(update_record); update_index++) {
    update_record rec = updates[update_index];
    display_float(rec.x, rec.y, rec.value, rec.width, rec.decimal_places);
  }
}

void count_pedal_switch() {
  pedal_ms = millis();
  if(pedal_ms > last_pedal_ms) { // ignore wrap-around
    speed.value = METERS_PER_CRANK/(pedal_ms-last_pedal_ms) * 3600; // m/ms = km/s * 3600 = km/h
    power.value = speed.value * speed.value * ((resistance + BASE_WORK)/(100 + BASE_WORK)) * RESISTANCE_TORQUE_MULTIPLIER /1000;
    }
  if ( trip_active ) {
    trip_distance.value += METERS_PER_CRANK;
  }
  last_pedal_ms = pedal_ms;
}

void setup() {
  pinMode (UP_BUTTON, INPUT_PULLUP);
  pinMode (DOWN_BUTTON, INPUT_PULLUP);
  pinMode (MOTOR_UP_PWM, OUTPUT);
  pinMode (MOTOR_DOWN_PWM, OUTPUT);
  pinMode (MAGNET_POSITION, INPUT);
  pinMode (CALIBRATION, INPUT_PULLUP); // pullup on analog for single ended control input
  pinMode (PEDAL_SWITCH, INPUT_PULLUP);
  pinMode (TRIP_RESET, INPUT_PULLUP);

  // Serial.begin(9600);
  // Serial.println();
  
  analogWrite(MOTOR_UP_PWM, 0);
  analogWrite(MOTOR_DOWN_PWM, 0);

  // apparently need to wait for the display to warm up on power-on if we have an eager microcontroller
  delay(500); 
  setup_lcd();
  
  // display the labels
  display_text(13,0,"% RES");
  display_text(0,2,"00:00:00");
  for (update_index=0; update_index < sizeof(updates)/sizeof(update_record); update_index++) {
    update_record rec = updates[update_index];
    display_text(rec.labelx, rec.labely, rec.label);
  }
}

void loop() {

  current_ms = millis();

  // this is just scaled from magnet position, and hard-calibrated to one machine, should clean this up
  resistance = round( (float)(analogRead(MAGNET_POSITION)-15.0)*(100.0/435.0) );

  // stop the motor if when no button is pressed
  if ( (digitalRead(DOWN_BUTTON) && digitalRead(UP_BUTTON)) || resistance <= 0 || resistance >= 100 ) {
    // digitalWrite(MOTOR_UP_PWM, 0);
    // digitalWrite(MOTOR_DOWN_PWM, 0);
    analogWrite(MOTOR_UP_PWM, 0);
    analogWrite(MOTOR_DOWN_PWM, 0);
  }
  // run the motor upward when the up button is pressed
  if ( !digitalRead(UP_BUTTON) && digitalRead(DOWN_BUTTON) && resistance < 100) {
    // digitalWrite(MOTOR_UP_PWM, 1);
    analogWrite(MOTOR_UP_PWM, UP_MOTOR_SPEED);
  }
  
  // run the motor downward when the down button is pressed
  if ( !digitalRead(DOWN_BUTTON) && digitalRead(UP_BUTTON) && resistance > 0 ) {
    // digitalWrite(MOTOR_DOWN_PWM, 1);
    analogWrite(MOTOR_DOWN_PWM, DOWN_MOTOR_SPEED);
  }

  // button press state machine, may need debounce
  if ( digitalRead(TRIP_RESET) && trip_reset_pressed) {
    trip_reset_pressed = false;
  }
  if ( !digitalRead(TRIP_RESET) && !trip_reset_pressed) {
    reset_trip();
    trip_reset_pressed = true;
  }

  // pedal switch state machine, may need debounce
  if ( digitalRead(PEDAL_SWITCH) && pedal_switch_closed ) {
    pedal_switch_closed = false;
  }
  if ( !digitalRead(PEDAL_SWITCH) && !pedal_switch_closed ) {
    count_pedal_switch();
    pedal_switch_closed = true;
  }

  // rollover-safe interval activation
  if (current_ms - last_update_ms  >= UPDATE_INTERVAL_MS) {
    last_update_ms += UPDATE_INTERVAL_MS;
    do_update();
  }
  
  /* diagnostic code for the position sensor and calibration potentiometer
  Serial.print("Position: ");
  Serial.println(analogRead(POSITION));
  Serial.println( (float)(analogRead(POSITION)-15.0)*(100.0/435.0) );
  Serial.print("Calibration: ");
  Serial.println(analogRead(CALIBRATION));
  Serial.print("SpeedSwitch: ");
  Serial.println(digitalRead(SPEED_SWITCH));
  */
  delay(10);
}
