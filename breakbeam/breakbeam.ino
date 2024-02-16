/*
  IR Breakbeam sensor demo!
  Updated to support 2 breakbeams
*/

#define LEDPIN 13
#define SENSORPIN1 4
#define SENSORPIN2 5

// variables will change:
int sensorState1 = 0, lastState1 = 0;
int sensorState2 = 0, lastState2 = 0;

void setup() {
  // initialize the LED pin as an output:
  pinMode(LEDPIN, OUTPUT);

  // initialize the sensor pin as an input:
  pinMode(SENSORPIN1, INPUT);
  digitalWrite(SENSORPIN1, HIGH);  // turn on the pullup1
  pinMode(SENSORPIN2, INPUT);
  digitalWrite(SENSORPIN2, HIGH);  // turn on the pullup2

  Serial.begin(9600);
}

void loop() {
  // read the state of the pushbutton value:
  sensorState1 = digitalRead(SENSORPIN1);
  sensorState2 = digitalRead(SENSORPIN2);

  // check if the sensor beam is broken
  // if it is, the sensorState is LOW:
  if (sensorState1 == LOW || sensorState2 == LOW) {
    // turn LED on:
    digitalWrite(LEDPIN, HIGH);
  } else {
    // turn LED off:
    digitalWrite(LEDPIN, LOW);
  }

  // Detect direction based on the order sensors are triggered
  if (sensorState1 == LOW && lastState2 == HIGH && lastState1 == HIGH) {
    Serial.println("Motion from Right to Left");
  } else if (sensorState2 == LOW && lastState1 == HIGH && lastState2 == HIGH) {
    Serial.println("Motion from Left to Right");
  }

  // Print "Broken" or "Unbroken" to the console for each sensor
  if (sensorState1 != lastState1) {
    Serial.print("Sensor 1 is ");
    Serial.println(sensorState1 == LOW ? "Broken" : "Unbroken");
  }
  if (sensorState2 != lastState2) {
    Serial.print("Sensor 2 is ");
    Serial.println(sensorState2 == LOW ? "Broken" : "Unbroken");
  }

  lastState1 = sensorState1;
  lastState2 = sensorState2;
}
