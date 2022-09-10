void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  while(Serial.available() > 0){  
    Serial.setTimeout(10);
    String s = Serial.readString();
    int len_s = s.length();
    for(int i= 0; i< len_s-2; i++){
//      Serial.write(s[i]+"X\n");
        Serial.print(s[i]);
        Serial.println('X');
    }
//    char b = Serial.read();
//    Serial.print(b);
//    Serial.println('X');
//    Serial.println(s);
//    Serial.println(len_s);
  }
}
