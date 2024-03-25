/*
  Sensor distance calculation modified from: https://www.instructables.com/Simple-Arduino-and-HC-SR04-Example/

  Using 2 HC-SR04 Ping distance sensors with doorway

*/

// first sensor pins
#define trigPin1 13
#define echoPin1 12

// second sensor pins
#define trigPin2 11
#define echoPin2 10

// tare button for zeroing the sensors at normal distance
#define tareButton 2

const int MAX_OCCUPANCY = 5; // change as needed

// debouncers and delays to change in testing
const long ENTRY_THRESHOLD = 50; // if person is less than 50cm they are present
const int DEBOUNCE_TIME = 750; // debounce time in ms (1000ms == 1s)
const int SENSOR_DELAY = 250; // delay between the break of the 1 and 2 sensors to count as an entry or exit

const int SENSOR_LOOP = 200; // Delay between the two sensors (may need to change)

// trackers for global events
unsigned long lastEventTime = 0; // last time an entry/exit event was recorded
int currentOccupancy;

long normalDistance1 = 0; // set with tare button
long normalDistance2 = 0; // set with tare button

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
}

void loop() 
{
  // zero button control that sets normal distance with no person in doorway
  if (digitalRead(tareButton) == LOW) {
    //calibrateSensors();
    //delay(50); // debouncer for button
  }

  // distance with the sensor1
  long distance1 = measureDistance(trigPin1, echoPin1);
  Serial.print("Occ: ");
  Serial.print(currentOccupancy);
  Serial.print(", Distance 1: ");
  Serial.print(distance1);
  Serial.println(" cm");
  
  // so the sensors alternate "on" time
  delay(SENSOR_LOOP); 
  
  // distance with sensor2
  long distance2 = measureDistance(trigPin2, echoPin2);
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


bool sensor1Triggered = false;
bool sensor2Triggered = false;


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

      if (currentOccupancy > MAX_OCCUPANCY) // if a person comes in when the occupancy is full
      { 
        // buzzer beep
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
      }
      lastEventTime = currentTime; // Update last event time for people debouncer
    }
  }
}