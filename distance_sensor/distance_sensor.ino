/*
  Sensor code originally modified from: https://www.instructables.com/Simple-Arduino-and-HC-SR04-Example/

  HC-SR04 Ping distance sensors with doorway

*/

// first sensor pins
#define trigPin1 13
#define echoPin1 12

// second sensor pins
#define trigPin2 11
#define echoPin2 10

// tare button for zeroing the sensors at normal distance
#define tareButton 2

long normalDistance1 = 0;
long normalDistance2 = 0;

int currentOccupancy;
const int MAX_OCCUPANCY = 15;

// debouncers for people
const long ENTRY_THRESHOLD = 50; // if person is less than 50cm they are present
const int DEBOUNCE_TIME = 2000; // debounce time in ms (1000ms == 1s)
unsigned long lastEventTime = 0; // last time an entry/exit event was recorded

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
    calibrateSensors();
    delay(50);    // debouncer
  }

  // distance with the sensor1
  long distance1 = measureDistance(trigPin1, echoPin1);
  Serial.print("Distance 1: ");
  Serial.print(distance1);
  Serial.println(" cm");
  
  // added delay since the sensors should alternate
  delay(250); 
  
  // distance with sensor2
  long distance2 = measureDistance(trigPin2, echoPin2);
  Serial.print("Distance 2: ");
  Serial.print(distance2);
  Serial.println(" cm");

  // Logic to determine entry or exit
  checkEntryOrExit(distance1, distance2);


  delay(250); // delay before next sensing cycle
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

void checkEntryOrExit(long distance1, long distance2)
// important code that checks if the distances currently being sensed count as an entry or exit or nothing 
{
  if (millis() - lastEventTime > DEBOUNCE_TIME) // cant have a new entry/exit in less than debounce time
  {
    if (distance1 < normalDistance1 && distance2 > normalDistance2) //entry detected
    {
      currentOccupancy++;
      Serial.print("Entry detected. Occupancy: ");
      Serial.println(currentOccupancy);
      if (currentOccupancy > MAX_OCCUPANCY) // Check against max occupancy
      { 
        Serial.println("max occupancy reached. mechanical actuation etc");
      }
      lastEventTime = millis(); // Update last event time for people debouncer
    } 
    else if (distance1 > normalDistance1 && distance2 < normalDistance2) // exit detected
    {
      if (currentOccupancy > 0) // no negative occupancy
      {
        currentOccupancy--;
        Serial.print("Exit detected. Occupancy: ");
        Serial.println(currentOccupancy);
      }
      lastEventTime = millis(); // Update last event time for people debouncer
    }
  }
}