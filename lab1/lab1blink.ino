void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, HIGH); // sets the digital pin 13 on
  delay(500);            // waits for a second
  digitalWrite(13, LOW);  // sets the digital pin 13 off
  delay(500);            // waits for a second
}
