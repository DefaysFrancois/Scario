#include <Arduino.h>

//Pin numbers definition
  //Ultrasound
const int trigPinFront = 3;
const int echoPinFront = 2;
  //IR
const int irPin = A0;
  //SERVO
const int servo_PIN = 4;
  //Motor Pin
const int motorLeftIN1 = 8;
const int motorLeftIN2 = 9;
const int motorRightIN3 = 10;
const int motorRightIN4 = 11;
const int motorEnableLeftEN12 = 12;
const int motorEnableRightEN34 = 13;


unsigned long delayTime = 150;
unsigned long currentTime = millis();
unsigned long previousTime = millis();

//Variables for Ultrasonic Sensors
long durationFront;
int distanceFront;
int distanceRight;
int distanceLeft;
const int minFrontDistance = 40;
const int stuckDistance = 10;


boolean exit_stuck = false;

//Variables for the Motors
const int leftMotorSpeed = 100; //best at 255
const int rightMotorSpeed = 110; //best at 255

//controle
enum Direction {FRONT, LEFT, RIGHT, BACK, STOP};
Direction directn;
Direction prev_directn;

//SERVO
#include <Servo.h>
Servo myservo;
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

//Methods Motor Control
void stopCar () {
  if (prev_directn != directn){
    Serial.print("|stop");
    prev_directn = directn;
  }
  analogWrite(motorEnableLeftEN12, 0);
  analogWrite(motorEnableRightEN34, 0);
}
void setLeftForward(){
  digitalWrite(motorLeftIN1, HIGH);
  digitalWrite(motorLeftIN2, LOW);
}
void setLeftBackward(){
  digitalWrite(motorLeftIN1, LOW);
  digitalWrite(motorLeftIN2, HIGH);
}
void setRightForward(){
  digitalWrite(motorRightIN3, LOW);
  digitalWrite(motorRightIN4, HIGH);
}
void setRightBackward(){
  digitalWrite(motorRightIN3, HIGH);
  digitalWrite(motorRightIN4, LOW);
}
void goBack () {
  Serial.print("|back");
  setLeftBackward();
  setRightBackward();
  analogWrite(motorEnableLeftEN12, leftMotorSpeed);
  analogWrite(motorEnableRightEN34, rightMotorSpeed);
}
void goForward () {
  Serial.print("|go forward");
  setRightForward();
  setLeftForward();
  analogWrite(motorEnableLeftEN12, leftMotorSpeed);
  analogWrite(motorEnableRightEN34, rightMotorSpeed);
}
void goLeft () {
  Serial.print("|go left");
  setLeftBackward();
  setRightForward();
  analogWrite(motorEnableLeftEN12, leftMotorSpeed);
  analogWrite(motorEnableRightEN34, rightMotorSpeed);
}
void goRight () {
  Serial.print("|go right");
  setLeftForward();
  setRightBackward();
  analogWrite(motorEnableLeftEN12, leftMotorSpeed);
  analogWrite(motorEnableRightEN34, rightMotorSpeed);
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
        delayTime = 700;
      }
      else
      {
        if (distanceRight >= distanceLeft){
          directn = RIGHT;
          delayTime = 700;
        }
        else
        {
          directn = LEFT;
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
  prev_directn = directn;
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
  pinMode(motorEnableLeftEN12, OUTPUT);
  pinMode(motorLeftIN1, OUTPUT);
  pinMode(motorLeftIN2, OUTPUT);
  pinMode(motorEnableRightEN34, OUTPUT);
  pinMode(motorRightIN3, OUTPUT);
  pinMode(motorRightIN4, OUTPUT);
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
  Serial.println("Setup OK");
}
void loop() {
  currentTime = millis();
  if (irrecv.decode(&results)) {
    irrecv.resume();
    if (results.value == FORW){
      onoff = 1;
      lookFront();
    }
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
    directn = STOP;
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
}