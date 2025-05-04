#include <Wire.h>

const int StepX = 2, DirX = 5;
const int StepY = 3, DirY = 6;
const int StepZ = 4, DirZ = 7;
const int StepA = 9, DirA = 10;

int stepDelay = 1000;
String currentCommand = "";

// Tilt data from MPU6050
float roll = 0.0, pitch = 0.0;

// Thresholds
const float pitchThreshold = 15;
const float rollThreshold = 15;
const float tolerance = 5;

// Timed correction logic
bool pitchCorrectionActive = false;
bool rollCorrectionActive = false;
unsigned long pitchTriggerTime = 0;
unsigned long rollTriggerTime = 0;
const unsigned long pullDuration = 500; // milliseconds

void setup() {
  pinMode(StepX, OUTPUT); pinMode(DirX, OUTPUT);
  pinMode(StepY, OUTPUT); pinMode(DirY, OUTPUT);
  pinMode(StepZ, OUTPUT); pinMode(DirZ, OUTPUT);
  pinMode(StepA, OUTPUT); pinMode(DirA, OUTPUT);

  Serial.begin(9600);
  Wire.begin(8);
  Wire.onReceive(receiveTiltData);

  Serial.println("CDPR Ready: Manual + Passive Control");
}

void receiveTiltData(int howMany) {
  String tilt = "";
  while (Wire.available()) {
    tilt += (char)Wire.read();
  }

  int sep = tilt.indexOf(',');
  if (sep != -1) {
    roll = tilt.substring(0, sep).toFloat();
    pitch = tilt.substring(sep + 1).toFloat();
    Serial.print("Roll: "); Serial.print(roll);
    Serial.print(" | Pitch: "); Serial.println(pitch);
  }
}

void applyDirection(bool dx, bool dy, bool dz, bool da) {
  digitalWrite(DirX, dx); digitalWrite(DirY, dy);
  digitalWrite(DirZ, dz); digitalWrite(DirA, da);
}

void stepMotors(int sx, int sy, int sz, int sa) {
  if (sx > 0) digitalWrite(StepX, HIGH);
  if (sy > 0) digitalWrite(StepY, HIGH);
  if (sz > 0) digitalWrite(StepZ, HIGH);
  if (sa > 0) digitalWrite(StepA, HIGH);
  delayMicroseconds(stepDelay);
  digitalWrite(StepX, LOW); digitalWrite(StepY, LOW);
  digitalWrite(StepZ, LOW); digitalWrite(StepA, LOW);
  delayMicroseconds(stepDelay);
}

void stopMotors() {
  digitalWrite(StepX, LOW); digitalWrite(StepY, LOW);
  digitalWrite(StepZ, LOW); digitalWrite(StepA, LOW);
}

// 1. Manual movement (keyboard input)
void handleUserMovement() {
  int stepVal = 15;
  if (currentCommand == "\u001B[C") { 
    applyDirection(HIGH, HIGH, LOW, LOW); 
    stepMotors(stepVal, stepVal, stepVal, stepVal);
  }
  else if (currentCommand == "\u001B[D") { 
    applyDirection(LOW, LOW, HIGH, HIGH);
    stepMotors(stepVal, stepVal, stepVal, stepVal);
  }
  else if (currentCommand == "\u001B[A") { 
    applyDirection(LOW, HIGH, LOW, HIGH);
    stepMotors(stepVal, stepVal, stepVal, stepVal);
  }
  else if (currentCommand == "\u001B[B") { 
    applyDirection(HIGH, LOW, HIGH, LOW); 
    stepMotors(stepVal, stepVal, stepVal, stepVal);
  }
  else if (currentCommand == "U") {
    applyDirection(LOW, LOW, LOW, LOW);
    stepMotors(stepVal, stepVal, stepVal, stepVal);
  }
  else if (currentCommand == "D") {
     applyDirection(HIGH, HIGH, HIGH, HIGH);
    stepMotors(stepVal, stepVal, stepVal, stepVal);
  }
  else {
    stopMotors();
  }
}

// 2. Automatic stabilization (triggered + timed)
void applyStabilization() {
  unsigned long now = millis();

  // --- PITCH correction (X, Y or Z, A pull) ---
  if (!pitchCorrectionActive && abs(pitch) >= pitchThreshold) {
    pitchCorrectionActive = true;
    pitchTriggerTime = now;
  }

  if (pitchCorrectionActive && (now - pitchTriggerTime <= pullDuration)) {
    if (pitch > pitchThreshold) {
      // Tilted right → X, Y pull
      applyDirection(LOW, LOW, LOW, LOW);
      stepMotors(2.5, 2.5, 0, 0);
    } else if (pitch < -pitchThreshold) {
      // Tilted left → Z, A pull
      applyDirection(LOW, LOW, LOW, LOW);
      stepMotors(0, 0, 2.5, 2.5);
    }
  } else if (pitchCorrectionActive && (now - pitchTriggerTime > pullDuration)) {
    pitchCorrectionActive = false;
  }

  // --- ROLL correction (X, A or Y, Z pull) ---
  if (!rollCorrectionActive && abs(roll) >= rollThreshold) {
    rollCorrectionActive = true;
    rollTriggerTime = now;
  }

  if (rollCorrectionActive && (now - rollTriggerTime <= pullDuration)) {
    if (roll > rollThreshold) {
      // Tilted back → X, A pull
      applyDirection(LOW, LOW, LOW, LOW);
      stepMotors(2.5, 0, 2.5, 0);
    } else if (roll < -rollThreshold) {
      // Tilted front → Y, Z pull
      applyDirection(LOW, LOW, LOW, LOW);
      stepMotors(0, 2.5, 0, 2.5);
    }
  } else if (rollCorrectionActive && (now - rollTriggerTime > pullDuration)) {
    rollCorrectionActive = false;
  }
}

void loop() {
  applyStabilization();  // Always running

  // Handle input
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    currentCommand = input;
  }

  handleUserMovement();  // Reacts to key press

  // Speed control
  if (currentCommand.startsWith("SPD-")) {
    stepDelay = currentCommand.substring(4).toInt();
    Serial.print("Speed set to delay = ");
    Serial.println(stepDelay);
    currentCommand = "";
  }
}
