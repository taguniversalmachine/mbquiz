const int ledPin = 1; // GPIO pin connected to the LED on the ESP32DevkitC
const int buttonPin = 2;
const int interchar_delay = 100;
const int short_delay = 500;
const int long_delay = 1000;
int button_state = LOW;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
 button_state = digitalRead(buttonPin);
 // while (button == HIGH) {
 // A();
 // B();
 // C();
 // D();
 // button = digitalRead(buttonPin);
 // }
 delay(short_delay);
 digitalWrite(ledPin, button_state);
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