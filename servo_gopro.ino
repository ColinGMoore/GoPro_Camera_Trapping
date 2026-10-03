// Code to control initiation and cessation of GoPro video recording using
// servo motor and GoPro Quick App.  Minimum recording time (TIME_DURATION)
// should be >= 10 seconds

#include <Servo.h>
Servo myServo;  // Create a servo object
// Pin Configurations
const int inputPin = 2;    // Digital pin connected to the input signal
const int outputPin = 13;   // Digital pin connected to the built-in LED (for testing)
// Note that the servo signal is attached to digital pin 6 - see line 25

// Time Constants (using UL for Unsigned Long precision)
// Set recording time (TIME_DURATION) in milliseconds in following line
const unsigned long TIME_DURATION = 10000UL; // 10 seconds in milliseconds (default)
const unsigned long PULSE_5_MS      = 5UL;      // 5 milliseconds pulse duration

// State Trackers
unsigned long lastInputHighTime = 0;   // Timestamp of the absolute last detected HIGH input
unsigned long pulseStartTime = 0;      // Timestamp of when the current 5ms pulse began
bool delayTimerRunning = false;        // Tracks if we are counting down the TIME_DURATION
bool pulseActive = false;              // Tracks if the output pin is currently HIGH
int lastInputState = LOW;              // Tracks the previous state of the input pin for edge detection

void setup() {
  myServo.attach(6);  // Attaches the servo signal to digital pin 6
  pinMode(inputPin, INPUT);            // Configure input pin (change to INPUT_PULLUP if needed)
  pinMode(outputPin, OUTPUT);          // Configure output pin
  digitalWrite(outputPin, LOW);        // Initialize output as LOW
  myServo.write(0);
}

void loop() {
  int currentInputState = digitalRead(inputPin);
  unsigned long currentMillis = millis();

  // 1. INPUT MONITORING WITH EDGE DETECTION
  // Check if the input just transitioned from LOW to HIGH (Rising Edge)
  if (currentInputState == HIGH && lastInputState == LOW) {
    
    if (!delayTimerRunning) {
      // FIRST TRIGGER: Timer is NOT running, so fire the immediate pulse
      myServo.write(60);  // Move to 60 degrees
      delay(500);        // Wait 500 milliseconds
      myServo.write(0);
      digitalWrite(outputPin, HIGH);
      pulseStartTime = currentMillis;
      pulseActive = true;
      
      // Start the TIME_DURATION window
      delayTimerRunning = true;
      lastInputHighTime = currentMillis;
    } else {
      // REPEATED TRIGGER DURING THE WINDOW: Do NOT fire a pulse.
      // Simply update the timestamp to reset the TIME_DURATION countdown.
      lastInputHighTime = currentMillis;
    }
  }
  
  // Save the current input state for the next loop iteration
  lastInputState = currentInputState;

  // 2. DELAY COUNTDOWN (SECOND PULSE)
  // If the TIME_DURATION countdown is active, wait until TIME_DURATION milliseconds of total quiet have passed.
  if (delayTimerRunning && !pulseActive) {
    if (currentMillis - lastInputHighTime >= TIME_DURATION) {
      // Fire the SECOND 5ms pulse after TIME_DURATION milliseconds of quiet
      digitalWrite(outputPin, HIGH);
      myServo.write(60);
      delay(500);
      myServo.write(0);
      pulseStartTime = currentMillis;   
      pulseActive = true;              
      delayTimerRunning = false;       // Reset the window timer
    }
  }

  // 3. PULSE DURATION TRACKER
  // Once the output goes HIGH (for either pulse), shut it down exactly 5ms later.
  if (pulseActive) {
    if (currentMillis - pulseStartTime >= PULSE_5_MS) {
      digitalWrite(outputPin, LOW);    // Turn output pin LOW
      myServo.write(0);
      pulseActive = false;             // Reset pulse state
    }
  }
}