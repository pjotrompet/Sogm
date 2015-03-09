/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
 int counter = 0
; //counter as test when no sensors are connected(initial)
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  float sensorValue = analogRead(A0);
  Serial.flush();
  counter = counter +1; //counting
  if(counter > 10){
    counter = 0;} //endif
  // print out the value you read:
  Serial.write(counter);
  //Serial.println(sensorValue);
  Serial.println(counter);
  delay(500);        // delay in between reads for stability
}
