const int ledPin = 4; // GPIO pin connected to the LED on the ESP32-CAM
const int interchar_delay = 100;
const int short_delay = 500;
const int long_delay = 1000;

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  A();
  B();
  C();
  D();
}

void pshort(){
  digitalWrite(ledPin, HIGH); // Turn the LED on
  delay(short_delay); // Wait for short delay
  digitalWrite(ledPin, LOW); // Turn the LED off
  delay(interchar_delay);
}

void plong(){
  digitalWrite(ledPin, HIGH); // Turn the LED on
  delay(long_delay); // Wait for long delay
  digitalWrite(ledPin, LOW); // Turn the LED off
  delay(interchar_delay);
}

void A() {
  pshort();
  plong();
}

void B(){
  plong();
  pshort();
  pshort();
  pshort();
}

void C(){
  plong();
  pshort();
  plong();
  pshort();
}

void D(){
  plong();
  pshort();
  pshort();
}