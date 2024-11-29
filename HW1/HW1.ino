// Pin definitions
const int trigPin = 9;      // HC-SR04 trigger pin
const int echoPin = 10;     // HC-SR04 echo pin
const int redLEDPin = 2;    // Red LED pin
const int greenLEDPin = 3;  // Green LED pin
const int buzzerPin = 8;    // Buzzer pin

float duration, distance;

void setup() {
  pinMode(trigPin, OUTPUT);   // Set trigger pin as output
  pinMode(echoPin, INPUT);    // Set echo pin as input
  pinMode(redLEDPin, OUTPUT); // Set red LED pin as output
  pinMode(greenLEDPin, OUTPUT); // Set green LED pin as output
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output

  Serial.begin(9600);  // Start serial communication at 9600 baud
}

void loop() {
  // Send trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo pulse duration and calculate distance in cm
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.0343) / 2;
  Serial.print("Distance: ");
  Serial.println(distance);

  // Control LEDs and buzzer based on distance
  if (distance < 20) {
    // Red LED on, green LED off
    digitalWrite(redLEDPin, HIGH);
    digitalWrite(greenLEDPin, LOW);

    // Buzzer continuously on, higher frequency (closer = more frequent)
    tone(buzzerPin, 2000);  // Higher frequency when very close

  } else if (distance >= 20 && distance <= 50) {
    // Green LED on, red LED off
    digitalWrite(redLEDPin, LOW);
    digitalWrite(greenLEDPin, HIGH);

    // Buzzer beeps intermittently, frequency based on distance
    int buzzerFrequency = map(distance, 20, 50, 1000, 500); // Adjust frequency based on distance
    tone(buzzerPin, buzzerFrequency);
    delay(500);  // Pause for a while to create intermittent sound
    noTone(buzzerPin);  // Turn buzzer off for a short period
    delay(500);

  } else {
    // Both LEDs off
    digitalWrite(redLEDPin, LOW);
    digitalWrite(greenLEDPin, LOW);

    // Buzzer off
    noTone(buzzerPin);
  }

  delay(100); // Short delay before repeating the loop
}
