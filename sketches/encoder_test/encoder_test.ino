/*
*Quadrature Decoder 
*/
#include "Arduino.h"
#include <digitalWriteFast.h>  // library for high performance reads and writes by jrraines
                               // see http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1267553811/0
                               // and http://code.google.com/p/digitalwritefast/
#include <TimerOne.h>

// It turns out that the regular digitalRead() calls are too slow and bring the arduino down when
// I use them in the interrupt routines while the motor runs at full speed.

// Quadrature encoders
//  encoder
#define c_EncoderInterruptA 0
#define c_EncoderInterruptB 1
#define c_EncoderPinA 2
#define c_EncoderPinB 3

volatile bool _EncoderASet;
volatile bool _EncoderBSet;
volatile bool _EncoderAPrev;
volatile bool _EncoderBPrev;
volatile long _EncoderTicks = 0;

volatile long CountsLast_20ms = 0;
volatile long CountsLast_200ms = 0;
volatile long CountsLast_1s = 0;
volatile long CountsLast_2s = 0;
volatile long TotalCounts = 0;

volatile byte TimerTickCount = 0;
volatile bool OneSecond = false;

int CountsPerRotation = 1000;
float DistancePerRotation = 1.57079;

void setup()
{
  Serial.begin(9600);

  // Quadrature encoders
  //  encoder
  pinMode(c_EncoderPinA, INPUT);      // sets pin A as input
  digitalWrite(c_EncoderPinA, LOW);  // turn on pullup resistors
  pinMode(c_EncoderPinB, INPUT);      // sets pin B as input
  digitalWrite(c_EncoderPinB, LOW);  // turn on pullup resistors
  attachInterrupt(c_EncoderInterruptA, HandleMotorInterruptA, CHANGE);
  attachInterrupt(c_EncoderInterruptB, HandleMotorInterruptB, CHANGE);
  Timer1.initialize(20000); // 20ms period
  Timer1.attachInterrupt(Handle20msTimerInterrupt, 20000);
  Serial.print("Note:Rotation calculation is based on ");
  Serial.print(CountsPerRotation, DEC);
  Serial.println(" counts per one full rotation");
  Serial.println("Counts:\t\t\t\t\tRPM:\t\t\t\tFPS:");
  Serial.println("20ms\t200ms\t1s\t2s\tTotal\t20ms\t200ms\t1s\t2s\t20ms\t200ms\t1s\t2s");
}

void loop()
{ 
  if (OneSecond){
    Serial.print(CountsLast_20ms, DEC);
    Serial.print("\t");
    Serial.print(CountsLast_200ms, DEC);
    Serial.print("\t");
    Serial.print(CountsLast_1s, DEC);
    Serial.print("\t");
    Serial.print(CountsLast_2s, DEC);
    Serial.print("\t");
    Serial.print(TotalCounts, DEC);
    Serial.print("\t");
    // RPM
    Serial.print(((float)CountsLast_20ms / (float)CountsPerRotation) * 3000.0, 4);
    Serial.print("\t");
    Serial.print(((float)CountsLast_200ms / (float)CountsPerRotation) * 300.0, 4);
    Serial.print("\t");
    Serial.print(((float)CountsLast_1s / (float)CountsPerRotation) * 60.0, 4);
    Serial.print("\t");
    Serial.print(((float)CountsLast_2s / (float)CountsPerRotation) * 30.0, 4);
    Serial.print("\t");
    // FPS
    Serial.print(((float)CountsLast_20ms / (float)CountsPerRotation) * 50.0 * DistancePerRotation, 4);
    Serial.print("\t");
    Serial.print(((float)CountsLast_200ms / (float)CountsPerRotation) * 5.0 * DistancePerRotation, 4);
    Serial.print("\t");
    Serial.print(((float)CountsLast_1s / (float)CountsPerRotation) * DistancePerRotation, 4);
    Serial.print("\t");
    Serial.print(((float)CountsLast_2s / (float)CountsPerRotation) * 0.5 * DistancePerRotation, 4);
    Serial.println();
    OneSecond = false;
  }
}

void Handle20msTimerInterrupt(){
  if ((TimerTickCount % 10) == 0) CountsLast_200ms = 0;
  if ((TimerTickCount % 50) == 0) {
    CountsLast_1s = 0;
    OneSecond = true;
  }
  if ((TimerTickCount % 100) == 0) {
    CountsLast_2s = 0;
    TimerTickCount = 0;
  }
  
  CountsLast_20ms = _EncoderTicks;
  CountsLast_200ms += _EncoderTicks;
  CountsLast_1s += _EncoderTicks;
  CountsLast_2s += _EncoderTicks;
  TotalCounts += _EncoderTicks;
  
  _EncoderTicks = 0;
  TimerTickCount++;
}

// Interrupt service routines for the  motor's quadrature encoder
void HandleMotorInterruptA(){
  _EncoderBSet = digitalReadFast(c_EncoderPinB);
  _EncoderASet = digitalReadFast(c_EncoderPinA);
  
  _EncoderTicks+=ParseEncoder();
  
  _EncoderAPrev = _EncoderASet;
  _EncoderBPrev = _EncoderBSet;
}

// Interrupt service routines for the right motor's quadrature encoder
void HandleMotorInterruptB(){
  // Test transition;
  _EncoderBSet = digitalReadFast(c_EncoderPinB);
  _EncoderASet = digitalReadFast(c_EncoderPinA);
  
  _EncoderTicks+=ParseEncoder();
  
  _EncoderAPrev = _EncoderASet;
  _EncoderBPrev = _EncoderBSet;
}

int ParseEncoder(){
  if(_EncoderAPrev && _EncoderBPrev){
    if(!_EncoderASet && _EncoderBSet) return 1;
    if(_EncoderASet && !_EncoderBSet) return -1;
  }else if(!_EncoderAPrev && _EncoderBPrev){
    if(!_EncoderASet && !_EncoderBSet) return 1;
    if(_EncoderASet && _EncoderBSet) return -1;
  }else if(!_EncoderAPrev && !_EncoderBPrev){
    if(_EncoderASet && !_EncoderBSet) return 1;
    if(!_EncoderASet && _EncoderBSet) return -1;
  }else if(_EncoderAPrev && !_EncoderBPrev){
    if(_EncoderASet && _EncoderBSet) return 1;
    if(!_EncoderASet && !_EncoderBSet) return -1;
  }
  return 0;
}


