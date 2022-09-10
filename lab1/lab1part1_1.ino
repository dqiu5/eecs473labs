void setup() {
  // put your setup code here, to run once:
  /*
  pinMode(12, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(5, OUTPUT);
  */

  DDRB = B10100;
  DDRD = B10100000;

  pinMode(A0, INPUT);

  //Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  digitalWrite(12, HIGH); // sets the digital pin         
  digitalWrite(10, HIGH);  // sets the digital pin 
  digitalWrite(7, HIGH); // sets the digital pin         
  digitalWrite(5, HIGH);  // sets the digital pin 
  */

  int voltage = analogRead(A0);
  
  if(voltage < 256){
    PORTB = B00000;
    PORTD = B00000000;
  }
  if(voltage >= 256){
    PORTB = B00000;
    PORTD = B00100000;
  }
  if(voltage >= 512){
    PORTB = B00000;
    PORTD = B10100000;
  }
  if(voltage >= 768){
    PORTB = B00100;
    PORTD = B10100000;
  }
  if(voltage == 1023){
    PORTB = B10100;
    PORTD = B10100000;
  }

  //Serial.println(voltage);
  
  

  
}
