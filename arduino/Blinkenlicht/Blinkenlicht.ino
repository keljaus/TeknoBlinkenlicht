// LED pins
#define PIN_RED 3
#define PIN_GREEN 5
#define PIN_BLUE 6

// Colorz
#define COLOR_NONE 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_BLUE 3
#define COLOR_YELLOW 4
#define COLOR_LILAC 5
#define COLOR_ORANGE 6
byte colorArctive = 0;

// Setup al stuff
void setup() {
  // Init RGB LED pins
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);

  // Start serial communication
  Serial.begin(9600);

  // Arduino Leonardo is a bit slow on opening Serial comms,
  // so give some 0.5 sec slack
  delay(500);

  setColor(COLOR_NONE);
}

// Le loop
void setColor(byte numColor) {
  switch (numColor) {
    case COLOR_NONE:
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);
      break;
    
    case COLOR_RED:
      analogWrite(PIN_RED, 16);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);
      break;
    
    case COLOR_GREEN:
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 16);
      analogWrite(PIN_BLUE, 0);
      break;
    
    case COLOR_BLUE:
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 16);
      break;
    
    case COLOR_YELLOW:
      analogWrite(PIN_RED, 10);
      analogWrite(PIN_GREEN, 9);
      analogWrite(PIN_BLUE, 0);
      break;
    
    case COLOR_LILAC:
      analogWrite(PIN_RED, 6);
      analogWrite(PIN_GREEN, 2);
      analogWrite(PIN_BLUE, 7);
      break;
    
    case COLOR_ORANGE:
      analogWrite(PIN_RED, 12);
      analogWrite(PIN_GREEN, 3);
      analogWrite(PIN_BLUE, 0);
      break;
  }
}

void loop() {
  // Check if there's an incoming request to blink a LED
  if (Serial.available() > 0) {
    byte numColor = Serial.read();
    setColor(numColor);
  }
}

