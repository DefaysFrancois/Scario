#include <Arduino.h>

//Pin numbers definition
  //Ultrasound
const int echoPinFront = 4;
const int trigPinFront = 5;
//SERVO
const int servo_PIN = 6;
//Motor Pin
const int motorLeftIN1 = 7;
const int motorLeftIN2 = 8;
const int motorRightIN3 = 9;
const int motorRightIN4 = 10;
const int motorEnableLeftEN12 = 11;
const int motorEnableRightEN34 = 12;


unsigned long delayTime = 150;
unsigned long currentTime = millis();
unsigned long previousTime = millis();

//Variables for Ultrasonic Sensors
long durationFront;
int distanceFront;
int distanceRight;
int distanceLeft;
const int minDistance = 40;
const int drivingDistance = 30;
const int stuckDistance = 10;

boolean exit_stuck = false;

//Variables for the Motors
const int leftMotorSpeed = 110; //best at 255
const int rightMotorSpeed = 110; //best at 255
const int leftMaxMotorSpeed = 255; //best at 255
const int rightMaxMotorSpeed = 255; //best at 255

//controle
enum Direction {FRONT, LEFT, RIGHT, BACK, STOP};
Direction directn;
Direction prev_directn;
int count_stuck = 0;
int max_count_stuck = 3;

//SERVO
#include <Servo.h>
Servo myservo;
int right_position = 0;
int front_position = 90;
int left_position = 180;

//Methods Motor Control
void stopCar () {
  if (prev_directn != directn){
    //Serial.print("|stop");
    prev_directn = directn;
  }
  digitalWrite(motorLeftIN1, LOW);
  digitalWrite(motorLeftIN2, LOW);
  digitalWrite(motorRightIN3, LOW);
  digitalWrite(motorRightIN4, LOW);
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
  //Serial.print("|back");
  setLeftBackward();
  setRightBackward();
  analogWrite(motorEnableLeftEN12, leftMotorSpeed);
  analogWrite(motorEnableRightEN34, rightMotorSpeed);
}
void goForward () {
  //Serial.print("|go forward");
  setRightForward();
  setLeftForward();
  analogWrite(motorEnableLeftEN12, leftMotorSpeed);
  analogWrite(motorEnableRightEN34, rightMotorSpeed);
}
void goLeft () {
  //Serial.print("|go left");
  setLeftBackward();
  setRightForward();
  analogWrite(motorEnableLeftEN12, leftMaxMotorSpeed);
  analogWrite(motorEnableRightEN34, rightMaxMotorSpeed);
}
void goRight () {
  //Serial.print("|go right");
  setLeftForward();
  setRightBackward();
  analogWrite(motorEnableLeftEN12, leftMaxMotorSpeed);
  analogWrite(motorEnableRightEN34, rightMaxMotorSpeed);
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
  //Serial.print(distanceRight);
  lookLeft();
  distanceLeft = sensorRead();
  //Serial.print(distanceLeft);
  lookFront();
  distanceFront = sensorRead();
  //Serial.print(distanceFront);
  //Serial.println("");
}
void chooseDirection(){
  distanceFront = sensorRead();
  count_stuck++;
  if (distanceFront <= minDistance || exit_stuck || count_stuck >= max_count_stuck){
    stopCar();
    measureDistance();
    if (distanceLeft <= minDistance && distanceRight <= minDistance && distanceFront <= minDistance){
      directn = BACK;
      exit_stuck = true;
      delayTime = 700;
    }
    else
    {
      if (distanceFront > minDistance && count_stuck < max_count_stuck){
        directn = FRONT;
        delayTime = 0;
      }
      else{
        if (distanceRight >= distanceLeft){
          directn = RIGHT;
          delayTime = 200;
        }
        else
        {
          directn = LEFT;
          delayTime = 200;
        }
      }
      exit_stuck = false;
    }
    count_stuck = 0;
  }
  else
  {
    directn = FRONT;
    delayTime = 0;
    count_stuck = 0;
  }
  prev_directn = directn;
/*   switch (directn)
  {
    case FRONT: break;
    case BACK: Serial.println("Go Back"); break;
    case LEFT: Serial.println("Go Left"); break;
    case RIGHT: Serial.println("Go Right"); break;
    default: break;
  } */
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
  Serial.begin(9600);
  delay(1000);
  myservo.attach(servo_PIN);
  Serial.println("Setup");
  lookRight();
  Serial.println("Looked Right");
  delay(200);
  lookLeft();
  Serial.println("Looked Left");
  delay(200);
  lookFront();
  Serial.println("Looking Front");
  Serial.println("Setup OK");
}
void loop() {
  currentTime = millis();
  switch (directn)
  {
    case FRONT: goForward(); break;
    case BACK: goBack(); break;
    case LEFT: goLeft(); break;
    case RIGHT: goRight(); break;
    default: break;
  }

  if (currentTime - previousTime > delayTime){
    chooseDirection();
    previousTime = millis();
  }
}