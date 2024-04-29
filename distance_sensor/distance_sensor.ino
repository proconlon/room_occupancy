/*
  This Arduino program uses two HC-SR04 ultrasonic sensors to manage room occupancy.
  It's designed to control entry to a room based on a predefined maximum occupancy.
  The sensors are angled apart so they can detect entry/exit direction independently.

  When a person approaches and breaks the beam of the first sensor, it triggers the system
  to check if they move past the second sensor, indicating an entry. Conversely, breaking
  the second sensor first would indicate an exit. The program uses debouncing by requiring
  a minimum time between successive entries/exits to avoid false triggers due to quick
  consecutive breaks of the sensor beams.

  Additional features include:
  - A calibration step that adjusts the ENTRY_THRESHOLD based on the normal detected 
    distance when the system starts up. If the normal distance is less than the default
    ENTRY_THRESHOLD (70 cm), it adjusts this threshold to 75% of the normal distance.
    This ensures the system works for a smaller doorway size or any obstuction less than the 70cm.
  - Visual indicators: LEDs and mechanical arm to alert when the room reaches maximum capacity.
  - An LCD display that shows the current number of occupants and indicates whether more
    people can enter or if the room is full.

  Adjustments can be made to the maximum occupancy, entry threshold, debounce time, and
  sensor delay to accommodate different environments and doorway sizes.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// first sensor pins
#define trigPin1 13
#define echoPin1 12
// second sensor pins
#define trigPin2 11
#define echoPin2 10

#define redLed 6 // 150 ohm resistor
#define greenLed 9
#define buzzerPin 7

// ******* CHANGE MAX OCCUPANCY AS NEEDED ******** //
const int MAX_OCCUPANCY = 3; 
// ****** CHANGE ENTRY_THRESHOLD AS NEEDED ******* //
long ENTRY_THRESHOLD = 70;      // if person is less than 70cm they are present and the beam is "broken"
// *********************************************** //

// debouncers and delays to change in testing
const int DEBOUNCE_TIME = 1500; // A new entry or exit cannot occur less than 1500 ms apart
const int SENSOR_DELAY = 2250;  // duration to wait for a second sensor to trigger after the first sensor is broken
const int SENSE_LOOP = 40;      // time between sensors (should be a small number to ensure continous sensing)

// trackers for global events
int currentOccupancy = 0;
bool isArmOut = false;
long distance1, distance2;
bool isDebouncing = false;
bool secondSensorActive = false;
bool firstSensorActive = false;
unsigned long lastDetectionTime = 0;
unsigned long firstSensorTime, secondSensorTime;

// set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// for stepper motor
const int ctr_a =2;
const int ctr_b =3;
const int ctr_c =4;
const int ctr_d =5;
const int STEP_TIME=1500; // Time delay in microseconds

void setup() {
  Serial.begin(9600);
  // Initialize pins for ultrasonic sensors
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  
  // stepper motor initialize
  pinMode(ctr_a,OUTPUT);
  pinMode(ctr_b,OUTPUT);
  pinMode(ctr_c,OUTPUT);
  pinMode(ctr_d,OUTPUT); 

  lcd.init();         // initialize the lcd
  lcd.backlight();    // Turn on the LCD screen backlight

  pinMode(buzzerPin, OUTPUT); // buzzer

  // init LEDs and turn on green
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  digitalWrite(greenLed, HIGH);

  lcdUpdate();
  delay(100);
  calibrateSensors(); // comment out if you don't care about calibration and just want to manage ENTRY_THRESHOLD by itself
}


void loop() {
    unsigned long currentTime = millis();
    static unsigned long lastCheckTime = 0;
    // Only check entry or exit if not currently debouncing
    if (!isDebouncing) {
      if (currentTime - lastCheckTime >= SENSE_LOOP) {
        checkEntryOrExit();
        lastCheckTime = currentTime;
      }
    } 
    else if (currentTime - lastDetectionTime > DEBOUNCE_TIME) 
    {
      // reset the debouncer after DEBOUNCE_TIME has passed
      isDebouncing = false;
    }
}

// If the normal distance is less than the default ENTRY_THRESHOLD
// this function updates it to be 75% of the "normal" distance with nobody in the doorway. 
// So for small doorways it will still work and not always detect a person 
// **** This means don't stand in front of the sensors when you power on the device ****
void calibrateSensors() 
{
  long normalDistance1 = measureDistance(trigPin1, echoPin1);
  long normalDistance2 = measureDistance(trigPin2, echoPin2);

  Serial.print("sensors normal distances: "); Serial.print(normalDistance1);
  Serial.print(" and "); Serial.println(normalDistance2);

  // Check if any normal distance is less than ENTRY_THRESHOLD
  if (normalDistance1 < ENTRY_THRESHOLD || normalDistance2 < ENTRY_THRESHOLD) {
    // Adjust the ENTRY_THRESHOLD to 75% of the smaller of the two normal distances
    ENTRY_THRESHOLD = 0.75 * min(normalDistance1, normalDistance2);
    Serial.print("Adjusted ENTRY_THRESHOLD to "); Serial.println(ENTRY_THRESHOLD);
  }
}

long measureDistance(int trigPin, int echoPin) 
{
  long duration, distance;
  // sonic pulses of 2/10
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH, 25000);  // 250000 microseconds as a timeout for the max range (400cm)
  if (duration == 0) {
    // no pulse was returned within the timeout period
    distance = -1;  // out of range
  } else {
    distance = duration * 0.034 / 2;  // distance calc
  }
  return distance;
}


// important code that checks if the distances currently being sensed count as an entry or exit or nothing 
void checkEntryOrExit() {
  distance1 = measureDistance(trigPin1, echoPin1);
  distance2 = measureDistance(trigPin2, echoPin2);

  Serial.print("Occ: "); Serial.print(currentOccupancy);
  Serial.print(" Sensor 1 Distance: "); Serial.print(distance1);
  Serial.print(", Sensor 2 Distance: "); Serial.println(distance2);

  if (!isDebouncing) {
    if (distance1 < ENTRY_THRESHOLD && !firstSensorActive && !secondSensorActive) { // if sensor 1 is broken and no sensor is active
      firstSensorActive = true;
      firstSensorTime = millis();
      Serial.println("First sensor triggered");
    } else if (distance2 < ENTRY_THRESHOLD && !secondSensorActive && !firstSensorActive) { // if sensor 2 is broken and no other sensor is active
      secondSensorActive = true;
      secondSensorTime = millis();
      Serial.println("Second sensor triggered");
    }
  }
  handleSensorEvents();
}

// handles the cases where both sensors have broken OR if nothing happened after one sensor was broken
void handleSensorEvents() {
  if (firstSensorActive && millis() - firstSensorTime < SENSOR_DELAY && distance2 < ENTRY_THRESHOLD) {
    entryDetected();
    firstSensorActive = false;
  }
  if (secondSensorActive && millis() - secondSensorTime < SENSOR_DELAY && distance1 < ENTRY_THRESHOLD) {
    exitDetected();
    secondSensorActive = false;
  }
  if (millis() - firstSensorTime >= SENSOR_DELAY) {
    firstSensorActive = false;
  }
  if (millis() - secondSensorTime >= SENSOR_DELAY) {
    secondSensorActive = false;
  }
}

void entryDetected() {
  currentOccupancy++;
  Serial.println("Entry detected.");
  lcdUpdate();
  isDebouncing = true;
  lastDetectionTime = millis();

  if (currentOccupancy >= MAX_OCCUPANCY) // if a person comes in when the occupancy is full
  { 
    // buzzer beep and move arm out
    audioWarning();
    delay(500);
    moveArmOut(); // will do nothing if the arm is already out
    digitalWrite(redLed, HIGH); // Turn on red led
    digitalWrite(greenLed, LOW); // Turn off green

    Serial.println("STOP COMING IN, MAX OCCUPANCY REACHED");
  }
  
}

void exitDetected() {
  isDebouncing = true;
  lastDetectionTime = millis();
  if (currentOccupancy > 0) // prevents negative occupancy
  {
    currentOccupancy--;
    Serial.println("Exit detected.");

    if (currentOccupancy < MAX_OCCUPANCY) { // if occupancy is less than max, let people in
      moveArmIn();
      digitalWrite(redLed, LOW);
      digitalWrite(greenLed, HIGH);
    }
    lcdUpdate();
  }
  
}

void audioWarning()
{
  // buzz an audio warning if you try to enter while room is at max occupancy
  // using non-blocking time so shouldn't stop sensors or anything else
  Serial.println("buzz buzz.");

  static bool buzzerActive = false; // static tracker
  static unsigned long buzzerStartTime = 0; // start time of buzzer
  const unsigned long buzzerDuration = 750; // buzz time in ms

  unsigned long currentMillis = millis(); // current time

  if (!buzzerActive) {
      buzzerActive = true;               // set buzzer active
      buzzerStartTime = currentMillis;   // record the start time
      digitalWrite(buzzerPin, HIGH);     // turn the buzzer on
  } else if (currentMillis - buzzerStartTime >= buzzerDuration) {
      digitalWrite(buzzerPin, LOW);      // turn the buzzer off after the duration
      buzzerActive = false;              // reset state
  }
}

void moveArmOut()
{
  if(!isArmOut) // don't move out if already out
  {
    isArmOut = true; 
    // move arm out
    // Move 90 degrees in one direction
    for(int i=128; i>=1; i--) { // 50 steps for 90 degrees
      digitalWrite(ctr_a,LOW); //A
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,LOW); //AB
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //B
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //BC
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //C
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //CD
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //D
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,LOW); //DA
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(STEP_TIME);
    }
    disableMotorCoils();

  }
}
void moveArmIn()
{
  // move the arm back only if it's out
  if(isArmOut) {
    isArmOut = false;
    // Move 90 degrees back to the original position
    for(int i=128; i>=1; i--) { // 50 steps back
      // Reverse the order of operations to step backwards
      digitalWrite(ctr_a,LOW); //DA
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //D
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //CD
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //C
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //BC
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,HIGH); //B
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,LOW); //AB
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
      digitalWrite(ctr_a,LOW); //A
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(STEP_TIME);
    }
    disableMotorCoils();
  }
}

// turns off power to the stepper
void disableMotorCoils() 
{
  digitalWrite(ctr_a, LOW);
  digitalWrite(ctr_b, LOW);
  digitalWrite(ctr_c, LOW);
  digitalWrite(ctr_d, LOW);
}

void lcdUpdate()
{
  lcd.setCursor(0, 0);
  lcd.print("  Occupants:    ");
  lcd.setCursor(14, 0);
  lcd.print(currentOccupancy);

  lcd.setCursor(0, 1);
  if (currentOccupancy >= MAX_OCCUPANCY) 
  {
    lcd.print("    ROOM FULL   "); // over write whole line
  } 
  else 
  {
    lcd.print("    Come in!    "); // clear line 
  }

}