/*
  Code modified from: https://www.instructables.com/Simple-Arduino-and-HC-SR04-Example/

  HC-SR04 Ping distance sensor with doorway

*/

#define trigPin 13
#define echoPin 12

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  long duration, distance;
  // sonic pulses of 2/10
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH); // Measure response pulse time
  distance = duration * 0.034 / 2; // distance calculation

  // Print the distance in cm
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Check if the distance is less than 91.44 cm == 36 inches
  // if they are within 3 cm from the sensor this does not count as a person.
  if (distance < 91.44 && distance > 3) {
    Serial.println("Person in doorway: true");
  } else {
    Serial.println("Person in doorway: false");
  }


  delay(500);
}