#include <Arduino.h>

//Pin numbers definition
const int motorEnableLeft = 9;
const int motorForwardLeft = 7;
const int motorBackLeft = 8;
const int motorEnableRight = 11;
const int motorForwardRight = 12;
const int motorBackRight = 10;
const int trigPinFront = 3;
const int echoPinFront = 2;
const int irPin = A0;

//Variables for the Motors
const int leftMotorSpeed = 255; //best at 255
const int rightMotorSpeed = 255;//best at 255

unsigned long delayTime = 150;
unsigned long moveTime = 700;
unsigned long currentTime = millis();
unsigned long previousTime = millis();

//Variables for Ultrasonic Sensors
long durationFront;
int distanceFront;
int distanceRight;
int distanceLeft;
const int minFrontDistance = 30;
const int stuckDistance = 10;

//controle
enum Direction {FRONT, LEFT, RIGHT, BACK};
Direction directn;
boolean exit_stuck = false;

//SERVO
#include <Servo.h>
Servo myservo;
const int servo_PIN = 4;
int right_position = 0;
int front_position = 90;
int left_position = 180;

//Variables for IR Sensor
#include <IRremote.h>
IRrecv irrecv(irPin);
decode_results results;
int onoff = 0;
//Control IR numbers
const long FORW = 16761405;
const long BACKW = 16720605;
const long ZERO = 16738455;

//Crazy
unsigned long crazyLookDelay;
unsigned long crazyMoveDelay;
unsigned long previousTimeCrazyMove = millis();
unsigned long previousTimeCrazyLook = millis();

void stopCar () {
  Serial.print("|stop");
  digitalWrite(motorForwardLeft, LOW);
  digitalWrite(motorBackLeft, LOW);
  digitalWrite(motorForwardRight, LOW);
  digitalWrite(motorBackRight, LOW);
  analogWrite(motorEnableLeft, 0);
  analogWrite(motorEnableRight, 0);
}
void setLeftForward(){
  digitalWrite(motorForwardLeft, HIGH);
  digitalWrite(motorBackLeft, LOW);
}
void setLeftBackward(){
  digitalWrite(motorForwardLeft, LOW);
  digitalWrite(motorBackLeft, HIGH);
}
void setRightForward(){
  digitalWrite(motorForwardRight, HIGH);
  digitalWrite(motorBackRight, LOW);
}
void setRightBackward(){
  digitalWrite(motorForwardRight, LOW);
  digitalWrite(motorBackRight, HIGH);
}
void goBack () {
  Serial.print("|back");
  setLeftBackward();
  setRightBackward();
  analogWrite(motorEnableLeft, leftMotorSpeed);
  analogWrite(motorEnableRight, rightMotorSpeed);
}
void goForward () {
  Serial.print("|go forward");
  setRightForward();
  setLeftForward();
  analogWrite(motorEnableLeft, leftMotorSpeed);
  analogWrite(motorEnableRight, rightMotorSpeed);
}
void goLeft () {
  Serial.print("|go left");
  setLeftBackward();
  setRightForward();
  analogWrite(motorEnableLeft, leftMotorSpeed);
  analogWrite(motorEnableRight, rightMotorSpeed);
}
void goRight () {
  Serial.print("|go right");
  setLeftForward();
  setRightBackward();
  analogWrite(motorEnableLeft, leftMotorSpeed);
  analogWrite(motorEnableRight, rightMotorSpeed);
}
int sensorRead () {
  //Read front sensor value
  digitalWrite(trigPinFront, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinFront, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinFront, LOW);
  durationFront = pulseIn(echoPinFront, HIGH);
  distanceFront = durationFront * 0.034 / 2;
  return(distanceFront);
}
void lookRight(){
  myservo.write(right_position);
  delay(500);
  Serial.print("|Right:");
}
void lookLeft(){
  myservo.write(left_position);
  delay(500);
  Serial.print("|Left:");
}
void lookFront(){
  myservo.write(front_position);
  delay(500);
  Serial.print("|Front:");
}
void measureDistance(){
  lookRight();
  distanceRight = sensorRead();
  Serial.print(distanceRight);
  lookLeft();
  distanceLeft = sensorRead();
  Serial.print(distanceLeft);
  lookFront();
  distanceFront = sensorRead();
  Serial.print(distanceFront);
}
void chooseDirection(){
  distanceFront = sensorRead();
  if (distanceFront <= minFrontDistance) {
    stopCar();
    measureDistance();
    if (distanceFront <= minFrontDistance || exit_stuck){
      if (distanceLeft <= minFrontDistance && distanceRight <= minFrontDistance && distanceFront <= minFrontDistance){
        directn = BACK;
        exit_stuck = true;
        moveTime = 700;
        delayTime = 700;
      }
      else
      {
        if (distanceRight >= distanceLeft){
          directn = RIGHT;
          moveTime = 700;
          delayTime = 700;
        }
        else
        {
          directn = LEFT;
          moveTime = 700;
          delayTime = 700;
        }
        exit_stuck = false;
      }
    }
    else {
      directn = FRONT;
      delayTime = 0;
    }
  }
  else{
    directn = FRONT;
    delayTime = 0;
  }
}
void getCrazyLook(){
  //look in a direction
  Serial.print("|Look");
  long servoAngle = random(0,180);
  myservo.write(servoAngle);
  delay(15);
  crazyLookDelay = random(100,1000);
}
void getCrazyMove(){
  //turn in a direction
  if (random(0,1000) < 500){
    directn = LEFT;
  }
  else{
    directn = RIGHT;
  }
  crazyMoveDelay = random(100,1000);
}
void setup() {
  pinMode(motorEnableLeft, OUTPUT);
  pinMode(motorForwardLeft, OUTPUT);
  pinMode(motorBackLeft, OUTPUT);
  pinMode(motorEnableRight, OUTPUT);
  pinMode(motorForwardRight, OUTPUT);
  pinMode(motorBackRight, OUTPUT);
  pinMode(trigPinFront, OUTPUT);
  pinMode(echoPinFront, INPUT);
  irrecv.enableIRIn();
  Serial.begin(9600);
  delay(1000);
  myservo.attach(servo_PIN);
  lookRight();
  delay(200);
  lookLeft();
  delay(200);
  lookFront();
  Serial.println("Setup K");
}
void loop() {
  currentTime = millis();
  if (irrecv.decode(&results)) {
    irrecv.resume();
    if (results.value == FORW)
      onoff = 1;
    else if (results.value == BACKW)
      onoff = 0;
    else if (results.value == ZERO)
      onoff = 2;
  }
  if (onoff == 1) {
    results.value = 0;
    switch (directn)
    {
      case FRONT: goForward(); break;
      case BACK: goBack(); break;
      case LEFT: goLeft(); break;
      case RIGHT: goRight(); break;
      default: break;
    }
    Serial.println();
    Serial.print("Current : ");
    Serial.print(currentTime);
    Serial.print(" | Previous : ");
    Serial.print(previousTime);
    Serial.print(" | Difference : ");
    Serial.print(currentTime - previousTime);
    Serial.print(" | delayTime : ");
    Serial.print(delayTime);
    Serial.println();
    if (currentTime - previousTime > delayTime){
      chooseDirection();
      previousTime = millis();
    }
  }
  else if (onoff == 0) {
    results.value = 0;
    stopCar();
  }
  else if (onoff == 2){
    results.value = 0;
    switch (directn)
    {
      case LEFT: goLeft(); break;
      case RIGHT: goRight(); break;
      default: break;
    }
    if (currentTime - previousTimeCrazyLook > crazyLookDelay){
      getCrazyLook();
      previousTimeCrazyLook = millis();
    }
    if (currentTime - previousTimeCrazyMove > crazyMoveDelay){
      getCrazyMove();
      previousTimeCrazyMove = millis();
    }
  }
  Serial.println();
}