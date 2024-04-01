/*
  Sensor distance calculation modified from: https://www.instructables.com/Simple-Arduino-and-HC-SR04-Example/

  Using 2 HC-SR04 Ping distance sensors with doorway

*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// first sensor pins
#define trigPin1 13
#define echoPin1 12
// second sensor pins
#define trigPin2 11
#define echoPin2 10

// tare button for zeroing the sensors at normal distance
#define tareButton 2

#define redLed 8 // 150 ohm resistor
#define greenLed 9

const int MAX_OCCUPANCY = 3; // change as needed

// debouncers and delays to change in testing
const long ENTRY_THRESHOLD = 70; // if person is less than 50cm they are present
const int DEBOUNCE_TIME = 2500; // debounce time in ms (1000ms == 1s)

const int SENSOR_LOOP = 100; // Delay between the two sensors (may need to change)




// trackers for global events
unsigned long lastEventTime = 0; // last time an entry/exit event was recorded
int currentOccupancy;
bool isArmOut = false;
long distance1, distance2;

long normalDistance1 = 0; // set with tare button
long normalDistance2 = 0; // set with tare button

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// for stepper motor
int pwm1=9;
int pwm2=10;
int ctr_a =2;
int ctr_b =3;
int ctr_c =4;
int ctr_d =5;
int sd =6;
int i=0;
int t=1500; // Time delay in microseconds

void setup() {
  Serial.begin(9600);
  // Initialize pins for sensor 1
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  
  // Initialize pins for sensor 2
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  
  pinMode(tareButton, INPUT);

  calibrateSensors();
  
  // stepper motor
  pinMode(ctr_a,OUTPUT);
  pinMode(ctr_b,OUTPUT);
  pinMode(ctr_c,OUTPUT);
  pinMode(ctr_d,OUTPUT); 

  lcd.init();         // initialize the lcd
  lcd.backlight();    // Turn on the LCD screen backlight

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  digitalWrite(greenLed, HIGH);


}

void loop() 
{
  lcdUpdate(); // update display every loop

  // zero button control that sets normal distance with no person in doorway
  if (digitalRead(tareButton) == LOW) {
    //calibrateSensors();
    //delay(50); // debouncer for button
  }

  // distance with the sensor1
  distance1 = measureDistance(trigPin1, echoPin1);
  Serial.print("Occ: ");
  Serial.print(currentOccupancy);
  Serial.print(", Distance 1: ");
  Serial.print(distance1);
  Serial.println(" cm");
    
  checkEntryOrExit(distance1, distance2);
  // so the sensors alternate "on" time
  delay(SENSOR_LOOP); 
  
  // distance with sensor2
  distance2 = measureDistance(trigPin2, echoPin2);
  Serial.print("Occ: ");
  Serial.print(currentOccupancy);
  Serial.print(", Distance 2: ");
  Serial.print(distance2);
  Serial.println(" cm");

  // Logic to determine entry or exit
  checkEntryOrExit(distance1, distance2);
  delay(SENSOR_LOOP);
}

void calibrateSensors() 
{
  normalDistance1 = measureDistance(trigPin1, echoPin1);
  normalDistance2 = measureDistance(trigPin2, echoPin2);
  Serial.println("sensors calibrated to ");
  Serial.println(normalDistance1);
  Serial.println(" and ");
  Serial.println(normalDistance2);
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
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // distance calculation
  
  return distance;
}

// important code that checks if the distances currently being sensed count as an entry or exit or nothing 
void checkEntryOrExit(long distance1, long distance2) {
  unsigned long currentTime = millis();
  // Check if a debounce period has elapsed since the last detected event
  if (currentTime - lastEventTime > DEBOUNCE_TIME) 
  {
    // Entry: Sensor 1 detects an object closer than the entry threshold
    if (distance1 < ENTRY_THRESHOLD && distance2 > ENTRY_THRESHOLD) 
    {
      currentOccupancy++;
      Serial.println("Entry detected.");

      if (currentOccupancy >= MAX_OCCUPANCY) // if a person comes in when the occupancy is full
      { 
        // buzzer beep
        // move mechanical arm to out
        audioWarning();
        moveArmOut();
        digitalWrite(redLed, HIGH); // Turn on red led
        digitalWrite(greenLed, LOW); // Turn off green

        Serial.println("STOP COMING IN, MAX OCCUPANCY REACHED");
      }
      lastEventTime = currentTime;
    }


    // Exit: Sensor 2 detects an object closer than the entry threshold
    else if (distance2 < ENTRY_THRESHOLD && distance1 > ENTRY_THRESHOLD) 
    {
      if (currentOccupancy > 0) // no negative occupancy
      {
        currentOccupancy--;
        
        Serial.println("Exit detected.");
        if (currentOccupancy < MAX_OCCUPANCY && isArmOut) {
          moveArmIn();
          digitalWrite(redLed, LOW);
          digitalWrite(greenLed, HIGH);

        }
      }
      lastEventTime = currentTime; // Update last event time for people debouncer
    }
  }
}

void audioWarning()
{
  // buzz an audio warning if you try to enter while room is at max occupancy
  Serial.println("buzz buzz.");
}

void moveArmOut()
{
  if(!isArmOut) // don't move out if already out
  {
    isArmOut = true; 
    // move arm out
    // Move 90 degrees in one direction
    for(i=127; i>=1; i--) { // 50 steps for 90 degrees
      digitalWrite(ctr_a,LOW); //A
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,LOW); //AB
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //B
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //BC
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //C
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //CD
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //D
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(t);
      digitalWrite(ctr_a,LOW); //DA
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(t);
    }
    disableMotorCoils();

  }
}
void moveArmIn()
{
  // move the arm back
    // Move 90 degrees back to the original position
    for(i=127; i>=1; i--) { // 50 steps back
      // Reverse the order of operations to step backwards
      digitalWrite(ctr_a,LOW); //DA
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //D
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //CD
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,LOW);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //C
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //BC
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,LOW);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,HIGH); //B
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,LOW); //AB
      digitalWrite(ctr_b,LOW);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
      digitalWrite(ctr_a,LOW); //A
      digitalWrite(ctr_b,HIGH);
      digitalWrite(ctr_c,HIGH);
      digitalWrite(ctr_d,HIGH);
      delayMicroseconds(t);
    }
  disableMotorCoils();
  isArmOut = false;
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
  lcd.print("Occupants:");
  lcd.print(currentOccupancy);

  lcd.setCursor(0, 1);
  if (currentOccupancy >= MAX_OCCUPANCY) 
  {
    lcd.print("   ROOM FULL   "); // over write whole line
  } 
  else 
  {
    lcd.print("   Come in!   "); // clear line 
  }

}