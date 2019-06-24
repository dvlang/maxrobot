/*
================================================================================

    File........... 
    Purpose........ 
    Author......... 
    E-mail......... 
    Started........ 
    Finished....... 
    Updated........ --/--/----
 
================================================================================
   Notes
================================================================================

================================================================================
  Updates
================================================================================
*/

//===============================================================================
//  Header Files
//===============================================================================

//===============================================================================
//  motor control
//===============================================================================

//Define some states for direction of movement
enum motionstate{
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  STOP
};

enum gripmotion{
  OPEN,
  CLOSE
};

enum armmotion{
  UP,
  DOWN
};

// Clockwise and counter-clockwise definitions.
// Depending on how you wired your motors, you may need to swap.
#define FWD  0
#define REV 1

#define DLY 10000

//Set an overall speed governor
#define SPD 127

// Motor definitions to make life easier:
//#define MOTOR_A 0
//#define MOTOR_B 1

// Pin Assignments //
//Default pins:
//#define LEFTDIR 12 // Direction control for motor A
//#define LEFTPWM 3  // PWM control (speed) for motor A

//my hookups
#define LEFTDIR 8 // Direction control for motor B
#define LEFTPWM 6 // PWM control (speed) for motor B

//
#define RIGHTDIR 13 // Direction control for motor B
#define RIGHTPWM 11 // PWM control (speed) for motor B

//

//===============================================================================
//  ARM AND GRIPPER
//===============================================================================

#include <Servo.h>
Servo servo1;  
Servo servo2; 
long num1,num2;  
long grippos,armpos;  


// We'll use SoftwareSerial to communicate with the XBee:
#include <SoftwareSerial.h>

//For Atmega328P's
// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
SoftwareSerial XBee(2, 3); // RX, TX

//===============================================================================
//  Constants
//===============================================================================
// Here we have a bunch of constants that will become clearer when we look at the
// readNesController() function. Basically, we will use these contents to clear
// a bit. These are chosen according to the table above.
const int A_BUTTON         = 0;
const int B_BUTTON         = 1;
const int SELECT_BUTTON    = 2;
const int START_BUTTON     = 3;
const int UP_BUTTON        = 4;
const int DOWN_BUTTON      = 5;
const int LEFT_BUTTON      = 6;
const int RIGHT_BUTTON     = 7;

//===============================================================================
//  Variables
//===============================================================================

//===============================================================================
//  Pin Declarations
//===============================================================================
//Inputs:


//Outputs:


//===============================================================================
//  Initialization
//===============================================================================
void setup() 
{
  // Initialize serial port speed for the serial terminal
    Serial.begin(9600);
    XBee.begin(9600);
  


    //DEFAULT MOTOR STATES
    setupArdumoto(); // Set all pins as outputs
    driveArdumoto(STOP);
  
    //arm
    initArm();
  
    //grip
    initGrip();

}

//===============================================================================
//  Main
//===============================================================================
void loop() 
{

  
  // Slight delay before we debug what was pressed so we don't spam the
  // serial monitor.
  delay(1);
  
 // handleSerial();

 //while (Serial.available() > 0) {
  while (XBee.available() > 0) {
     //char incomingCharacter = Serial.read();
     char incomingCharacter = XBee.read();
     //Serial.println(incomingCharacter); 

     switch (incomingCharacter) {
     
       
       case 'f':
            //Serial.println("commanded to move forwards"); 
            driveArdumoto(FORWARD); 
            break;
   
       case 'b':
            driveArdumoto(BACKWARD);
            //Serial.println("commanded to move backwards"); 
            break;
  
        case 'r':
            driveArdumoto(RIGHT);
            //Serial.println("commanded to move right"); 
            break;
  
        case 'l':
            driveArdumoto(LEFT);
            //Serial.println("commanded to move left"); 
            break;
  
        case 'o':
            adjustGripper(OPEN);
            //Serial.println("commanded to move open"); 
            break;
  
         case 'c':
            adjustGripper(CLOSE);
            //Serial.println("commanded to move close"); 
            break;
  
         case 'u':
            adjustArm(UP);
            //Serial.println("commanded to move up"); 
            break;
  
         case 'd':
            adjustArm(DOWN);
            //Serial.println("commanded to move down"); 
            break;
  
         case 's':
            driveArdumoto(STOP);
            //Serial.println("commanded to move stop"); 
            break;
    
         default:
            driveArdumoto(STOP);
            break;
  
      }
  }

  
  
  
}

//===============================================================================
//  Functions
//===============================================================================



//MOTOR DRIVER FUNCTIONS
// driveArdumoto drives 'motor' in 'dir' direction at 'spd' speed
void driveArdumoto(motionstate waytogo)
{
  if (waytogo == FORWARD)
  {
    //Serial.println("FORWARD");
    digitalWrite(LEFTDIR, REV);
    digitalWrite(RIGHTDIR, FWD);
    analogWrite(LEFTPWM, SPD);
    analogWrite(RIGHTPWM, SPD);
  }
  else if (waytogo == BACKWARD)
  {
  //  Serial.println("BACKWARD");
    digitalWrite(LEFTDIR, FWD);
    digitalWrite(RIGHTDIR, REV);
    analogWrite(LEFTPWM, SPD);
    analogWrite(RIGHTPWM, SPD);
  }  
    else if (waytogo == LEFT)
  {
    //Serial.println("LEFT");
    digitalWrite(LEFTDIR, REV);
    digitalWrite(RIGHTDIR, REV);
    analogWrite(LEFTPWM, SPD);
    analogWrite(RIGHTPWM, SPD);
  }  
    else if (waytogo == RIGHT)
  {
  //  Serial.println("RIGHT");
    digitalWrite(LEFTDIR, FWD);
    digitalWrite(RIGHTDIR, FWD);
    analogWrite(LEFTPWM, SPD);
    analogWrite(RIGHTPWM, SPD);
  }  
    else if (waytogo == STOP)
  {
    digitalWrite(LEFTDIR, FWD);
    digitalWrite(RIGHTDIR, REV);
    analogWrite(LEFTPWM, 0);
    analogWrite(RIGHTPWM, 0);
  }  
}

// stopArdumoto makes a motor stop
void stopArdumoto(byte motor)
{
  driveArdumoto(STOP);
}

// setupArdumoto initialize all pins
void setupArdumoto()
{
  Serial.println("Motor Init");  
  
  // All pins should be setup as outputs:
  pinMode(LEFTPWM, OUTPUT);
  pinMode(RIGHTPWM, OUTPUT);
  pinMode(LEFTDIR, OUTPUT);
  pinMode(RIGHTDIR, OUTPUT);

  // Initialize all pins as low:
  digitalWrite(LEFTPWM, LOW);
  digitalWrite(RIGHTPWM, LOW);
  digitalWrite(LEFTDIR, LOW);
  digitalWrite(RIGHTDIR, LOW);
}


// adjustGripper 
void adjustGripper(gripmotion dirtomove)
{
 // Serial.println("called adjust grip"); 
  if(dirtomove==OPEN){
     if (grippos <=180)
     {
        grippos+=5;
        servo2.write(grippos);
       // Serial.println(grippos);   
     }
  }
  else if (dirtomove==CLOSE){
     if (grippos >=102)
     {
        grippos-=5;
        servo2.write(grippos);
       // Serial.println(grippos);   
     }
  }
  
}

// adjustArm 
void adjustArm(armmotion dirtomove)
{
 // Serial.println("called adjust arm"); 
  if(dirtomove==UP){
     if (armpos <=85)
     {
        armpos+=5;
        servo1.write(armpos);
       // Serial.println(armpos);   
     }
  }
  else if (dirtomove==DOWN){
     if (armpos >=15)
     {
        armpos-=5;
        servo1.write(armpos);
        //Serial.println(armpos);  
     }
  }
  
}

// stopArdumoto makes a motor stop
void initGrip(void)
{
    Serial.println("Grip Init");  
    grippos=97;
    servo2.attach(10); //grip
    servo2.write(grippos);
}

// stopArdumoto makes a motor stop
void initArm(void)
{
    Serial.println("Arm Init");  
    servo1.attach(9); //arm
    armpos=9;
    servo1.write(armpos);
}

/*void handleSerial() {
  //while (Serial.available() > 0) {
  while (XBee.available() > 0) {
     //char incomingCharacter = Serial.read();
     char incomingCharacter = XBee.read();
     //Serial.println(incomingCharacter); 

     switch (incomingCharacter) {
     
       
       case 'f':
            //Serial.println("commanded to move forwards"); 
            driveArdumoto(FORWARD); 
            break;
   
       case 'b':
            driveArdumoto(BACKWARD);
            //Serial.println("commanded to move backwards"); 
            break;
  
        case 'r':
            driveArdumoto(RIGHT);
            //Serial.println("commanded to move right"); 
            break;
  
        case 'l':
            driveArdumoto(LEFT);
            //Serial.println("commanded to move left"); 
            break;
  
        case 'o':
            adjustGripper(OPEN);
            //Serial.println("commanded to move open"); 
            break;
  
         case 'c':
            adjustGripper(CLOSE);
            //Serial.println("commanded to move close"); 
            break;
  
         case 'u':
            adjustArm(UP);
            //Serial.println("commanded to move up"); 
            break;
  
         case 'd':
            adjustArm(DOWN);
            //Serial.println("commanded to move down"); 
            break;
  
         case 's':
            driveArdumoto(STOP);
            //Serial.println("commanded to move stop"); 
            break;
    
         default:
            driveArdumoto(STOP);
            break;
  
      }
  }
}*/
