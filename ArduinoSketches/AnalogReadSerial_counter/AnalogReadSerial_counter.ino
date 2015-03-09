/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
 int counter = 90; //counter as test when no sensors are connected(initial)
void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(215200);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue1 = analogRead(A0);
  int sensorValue2 = analogRead(A1);
  int sensors[]={sensorValue1, sensorValue2};
  counter = counter + 1; //make it count
  if(counter > 100){
    counter = 90;} //endif
    
  // print out the value you read:
  //Serial.write(sensorValue1);
  Serial.write(sensorValue1);
  Serial.write(001);
  Serial.write(sensorValue2);
  //Serial.println(sensorValue);
  //Serial.print(DEC <- 97);
  delay(1);        // delay in between reads for stability
}
