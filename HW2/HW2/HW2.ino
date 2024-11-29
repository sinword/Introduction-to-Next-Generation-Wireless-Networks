#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define MAX_CARDS 10 // Maximum number of cards: 10

MFRC522 rfid(SS_PIN, RST_PIN); // Instantiate rfid

const int greenLEDPin = 6;
const int redLEDPin = 7;

// Structure to store authorized IDs
struct {
  String ids[MAX_CARDS];
  int count = 0;
} authorizedCards;

bool commandMode = false;
String initialID = "";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);

  digitalWrite(greenLEDPin, LOW);
  digitalWrite(redLEDPin, LOW);
}

// Read RFID card ID
String readRFIDCard() {
  String rfidID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    rfidID += String(rfid.uid.uidByte[i], HEX); // Read RFID ID
  }
  rfidID.toUpperCase();
  return rfidID;
}

// Check if ID is authorized
bool isAuthorized(String id) {
  for (int i = 0; i < authorizedCards.count; i++) {
    if (authorizedCards.ids[i] == id) {
      return true;
    }
  }
  return false;
}

// Add authorized ID
bool addCard(String id) {
  if (authorizedCards.count >= MAX_CARDS) {
    Serial.println("Error: Authorization list is full");
    return;
  }
  for (int i = 0; i < authorizedCards.count; i++) {
    if (authorizedCards.ids[i] == id) {
      Serial.println("Id already exists: " + id);
      return;
    }
  }
  authorizedCards.ids[authorizedCards.count++] = id;
  Serial.println("ID added successfully: " + id);
  return true;
}

// Remove authorized ID
bool removeCard(int index) {
  String id = "";
  if (index < 0 || index >= authorizedCards.count) {
    Serial.println("Error: Invalid index number");
    return false;
  }
  id = authorizedCards.ids[index - 1];
  for (int i = index; i < authorizedCards.count - 1; i++) {
    authorizedCards.ids[i] = authorizedCards.ids[i + 1];
  }
  authorizedCards.count--;
  Serial.println("ID removed successfully: " + id);
  return true;
}

// Display list of authorized IDs
void listCards() {
  Serial.println("Authorized ID List:");
  Serial.print("Total count: ");
  Serial.println(authorizedCards.count);
  for (int i = 0; i < authorizedCards.count; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(authorizedCards.ids[i]);
  }
  Serial.println("-------------------");
}

void processCommand(String command) {
  command.trim();

  if (command.startsWith("ADD")) {
    Serial.println("Please scan the card to add...");
    while(!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
      delay(100);
    }
    String newID = readRFIDCard();
    addCard(newID);
    commandMode = false;
    delay(1000);
  }
  else if (command == "LIST") {
    listCards();
    commandMode = false;
  }
  else if (command.startsWith("REMOVE")) {
    int index = command.substring(7).toInt() - 1;
    removeCard(index);
    commandMode = false;
  }
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardID = readRFIDCard();
    Serial.println("ID Read: " + cardID);
    if (initialID == "") {
      initialID = cardID;
      Serial.println("Initial authorized ID set successfully");
      addCard(initialID);
      digitalWrite(greenLEDPin, HIGH);
      delay(1000);
      digitalWrite(greenLEDPin, LOW);
    }
    // Check authorization
    else if (isAuthorized(cardID)) {
      digitalWrite(greenLEDPin, HIGH);
      delay(1000);
      digitalWrite(greenLEDPin, LOW);
      commandMode = true;
      Serial.println("Entering command mode");
      Serial.println("Available commands: ADD, LIST, REMOVE [number]");
    }
    else {
      digitalWrite(redLEDPin, HIGH);
      delay(1000);
      digitalWrite(redLEDPin, LOW);
      Serial.println("Unauthorized ID");
    }
    rfid.PICC_HaltA();
  }

  // Process serial commands
  if (commandMode && Serial.available()) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }
}
