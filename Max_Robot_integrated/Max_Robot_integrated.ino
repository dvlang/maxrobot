/*
================================================================================

    File........... NES Controller Test Code
    Purpose........ To demonstrate how to interface to an NES controller
    Author......... Joseph Corleto
    E-mail......... corleto.joseph @ gmail.com
    Started........ 04/13/2016
    Finished....... 04/14/2016
    Updated........ --/--/----
 
================================================================================
   Notes
================================================================================
- The NES controller contains one 8-bit 4021 shift register inside. 

- This register takes parallel inputs and converts them into a serial output.

- This code first latches the data and then shifts in the first bit on the data line. 
  Then it clocks and shifts in on the data line until all bits are received.
  
- What is debugged are the button states of the NES controller.

- A logical "1" means the button is not pressed. A logical "0" means the button is
  pressed.
  
- This code shifts the first bit of data into the LSB.

- The order of shifting for the buttons is shown in the table below:

        Bit# | Button   
        --------------
          0  |   A  
        --------------
          1  |   B  
        --------------
          2  | Select   
        --------------
                      3  | Start  
        --------------
          4  |   Up  
        --------------
          5  |  Down  
        --------------
          6  |  Left   
        --------------
          7  | Right   
        --------------
        
- The NES controller pinout is shown below (looking into controllers
  connector end):
    __________
   /      |
  /       O 1 | 1 - Ground
        |           | 2 - Clock
  | 7 O   O 2 |   3 - Latch
  |           | 4 - Data Out
  | 6 O   O 3 | 5 - No Connection
  |           |   6 - No Connection
  | 5 O   O 4 |   7 - +5V
  |___________|

- Please visit http://www.allaboutcircuits.com to search for complete article!

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
byte nesRegister  = 0;    // We will use this to hold current button states

bool apressed=false;

int pwmValue,pwmMax;

//===============================================================================
//  Pin Declarations
//===============================================================================
//Inputs:
int nesData       = 4;    // The data pin for the NES controller

//Outputs:
int nesClock      = 2;    // The clock pin for the NES controller
int nesLatch      = 3;    // The latch pin for the NES controller

//===============================================================================
//  Initialization
//===============================================================================
void setup() 
{
  // Initialize serial port speed for the serial terminal
  Serial.begin(9600);
    XBee.begin(9600);
  
  // Set appropriate pins to inputs
  pinMode(nesData, INPUT);
  
  // Set appropriate pins to outputs
  pinMode(nesClock, OUTPUT);
  pinMode(nesLatch, OUTPUT);
  
  // Set initial states
  digitalWrite(nesClock, LOW);
  digitalWrite(nesLatch, LOW);

  //DEFAULT MOTOR STATES
  setupArdumoto(); // Set all pins as outputs

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
  // This function call will return the states of all NES controller's register
  // in a nice 8 bit variable format. Remember to refer to the table and
  // constants above for which button maps where!
  nesRegister = readNesController();
  
  // Slight delay before we debug what was pressed so we don't spam the
  // serial monitor.
  delay(180);
  
  // To give you an idea on how to use this data to control things for your
  // next project, look through the serial terminal code below. Basically,
  // just choose a bit to look at and decide what to do whether HIGH (not pushed)
  // or LOW (pushed). What is nice about this test code is that we mapped all
  // of the bits to the actual button name so no useless memorizing!

  /*  
  if (bitRead(nesRegister, A_BUTTON) == 0){
        //Serial.println("ARM AND GRIP ENABLE");
          apressed = true;
  }
  else if(bitRead(nesRegister, A_BUTTON) == 1){
    apressed=false;
  }
  else{}

    
  if (bitRead(nesRegister, B_BUTTON) == 0){
        Serial.println("NOT DEFINED");
  }

    
  if (bitRead(nesRegister, START_BUTTON) == 0){
        Serial.println("LIGHTS ON");
  }

  
  if (bitRead(nesRegister, SELECT_BUTTON) == 0){
        Serial.println("BEEP BEEP");
  }
*/
 handleSerial();



    driveArdumoto(STOP); 
  //move forward if B button isn't pressed  
  if (bitRead(nesRegister, UP_BUTTON) == 0){
 //      if(bitRead(nesRegister,A_BUTTON==0)){
       if(apressed){
          Serial.println("ARM UP");   
          adjustArm(UP);
       }
       else{
          Serial.println("MOVE FORWARD");
          driveArdumoto(FORWARD); 
       }
   }
    
    //move  backwards if B button isn't pressed
  if (bitRead(nesRegister, DOWN_BUTTON) == 0){
       if(apressed){
          Serial.println("ARM DOWN");   
          adjustArm(DOWN);
       }
       else{
          Serial.println("MOVE BACKWARDS");
          driveArdumoto(BACKWARD); 
       }
  }

    
  if (bitRead(nesRegister, LEFT_BUTTON) == 0){
    if(apressed){
      Serial.println("OPEN GRIP");  
      adjustGripper(OPEN);
    }
    else  {
      Serial.println("TURN LEFT");  
      driveArdumoto(LEFT);
    }
  }
  
  if (bitRead(nesRegister, RIGHT_BUTTON) == 0){
    if(apressed){
       Serial.println("CLOSE GRIP");  
       adjustGripper(CLOSE);
    }
    else  {
      Serial.println("TURN RIGHT ");
      driveArdumoto(RIGHT); 
    }
  }
}

//===============================================================================
//  Functions
//===============================================================================
///////////////////////
// readNesController //
///////////////////////
byte readNesController() 
{  
  // Pre-load a variable with all 1's which assumes all buttons are not
  // pressed. But while we cycle through the bits, if we detect a LOW, which is
  // a 0, we clear that bit. In the end, we find all the buttons states at once.
  int tempData = 255;
    
  // Quickly pulse the nesLatch pin so that the register grab what it see on
  // its parallel data pins.
  digitalWrite(nesLatch, HIGH);
  digitalWrite(nesLatch, LOW);
 
  // Upon latching, the first bit is available to look at, which is the state
  // of the A button. We see if it is low, and if it is, we clear out variable's
  // first bit to indicate this is so.
  if (digitalRead(nesData) == LOW)
    bitClear(tempData, A_BUTTON);
    
  // Clock the next bit which is the B button and determine its state just like
  // we did above.
  digitalWrite(nesClock, HIGH);
  digitalWrite(nesClock, LOW);
  if (digitalRead(nesData) == LOW)
    bitClear(tempData, B_BUTTON);
  
  // Now do this for the rest of them!
  
  // Select button
  digitalWrite(nesClock, HIGH);
  digitalWrite(nesClock, LOW);
  if (digitalRead(nesData) == LOW)
    bitClear(tempData, SELECT_BUTTON);

  // Start button
  digitalWrite(nesClock, HIGH);
  digitalWrite(nesClock, LOW);
  if (digitalRead(nesData) == LOW)
    bitClear(tempData, START_BUTTON);

  // Up button
  digitalWrite(nesClock, HIGH);
  digitalWrite(nesClock, LOW);
  if (digitalRead(nesData) == LOW)
    bitClear(tempData, UP_BUTTON);
    
  // Down button
  digitalWrite(nesClock, HIGH);
  digitalWrite(nesClock, LOW);
  if (digitalRead(nesData) == LOW)
    bitClear(tempData, DOWN_BUTTON);

  // Left button
  digitalWrite(nesClock, HIGH);
  digitalWrite(nesClock, LOW);
  if (digitalRead(nesData) == LOW)
    bitClear(tempData, LEFT_BUTTON);  
    
  // Right button
  digitalWrite(nesClock, HIGH);
  digitalWrite(nesClock, LOW);
  if (digitalRead(nesData) == LOW)
    bitClear(tempData, RIGHT_BUTTON);
    
  // After all of this, we now have our variable all bundled up
  // with all of the NES button states.*/
  return tempData;
}

//MOTOR DRIVER FUNCTIONS
// driveArdumoto drives 'motor' in 'dir' direction at 'spd' speed
void driveArdumoto(motionstate waytogo)
{
  if (waytogo == FORWARD)
  {
    Serial.println("FORWARD");
    digitalWrite(LEFTDIR, REV);
    digitalWrite(RIGHTDIR, FWD);
    analogWrite(LEFTPWM, SPD);
    analogWrite(RIGHTPWM, SPD);
  }
  else if (waytogo == BACKWARD)
  {
    Serial.println("BACKWARD");
    digitalWrite(LEFTDIR, FWD);
    digitalWrite(RIGHTDIR, REV);
    analogWrite(LEFTPWM, SPD);
    analogWrite(RIGHTPWM, SPD);
  }  
    else if (waytogo == LEFT)
  {
    Serial.println("LEFT");
    digitalWrite(LEFTDIR, REV);
    digitalWrite(RIGHTDIR, REV);
    analogWrite(LEFTPWM, SPD);
    analogWrite(RIGHTPWM, SPD);
  }  
    else if (waytogo == RIGHT)
  {
    Serial.println("RIGHT");
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

  if(dirtomove==OPEN){
     if (grippos <=180)
     {
        grippos+=5;
        servo2.write(grippos);
        Serial.println(grippos);   
     }
  }
  else if (dirtomove==CLOSE){
     if (grippos >=102)
     {
        grippos-=5;
        servo2.write(grippos);
        Serial.println(grippos);   
     }
  }
  
}

// adjustArm 
void adjustArm(armmotion dirtomove)
{

  if(dirtomove==UP){
     if (armpos <=85)
     {
        armpos+=5;
        servo1.write(armpos);
        Serial.println(armpos);   
     }
  }
  else if (dirtomove==DOWN){
     if (armpos >=15)
     {
        armpos-=5;
        servo1.write(armpos);
        Serial.println(armpos);  
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

void handleSerial() {
 //while (Serial.available() > 0) {
   while (XBee.available() > 0) {
   //char incomingCharacter = Serial.read();
   char incomingCharacter = XBee.read();
 //  int pwmValue,pwmMax;
   switch (incomingCharacter) {
     case 'f':
      pwmValue = pwmValue + 5;
      Serial.println("commanded to move forwards"); 
      driveArdumoto(FORWARD); 
      if(pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;
 
     case 'b':
      pwmValue = pwmValue - 5;
      driveArdumoto(BACKWARD);
      Serial.println("commanded to move backwards"); 
      if (pwmValue <= 0)
         pwmValue = 0;
      break;

      case 'r':
      pwmValue = pwmValue + 5;
      driveArdumoto(RIGHT);
      Serial.println("commanded to move right"); 
      if(pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;

       case 'l':
      pwmValue = pwmValue + 5;
       driveArdumoto(LEFT);
      Serial.println("commanded to move left"); 
      if(pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;

      case 'o':
      pwmValue = pwmValue + 5;
      adjustGripper(OPEN);
      Serial.println("commanded to move open"); 
      if(pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;

       case 'c':
      pwmValue = pwmValue + 5;
      adjustGripper(CLOSE);
      Serial.println("commanded to move close"); 
      if(pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;

       case 'u':
      pwmValue = pwmValue + 5;
      adjustArm(UP);
      Serial.println("commanded to move up"); 
      if(pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;

       case 'd':
      pwmValue = pwmValue + 5;
      adjustArm(DOWN);
      Serial.println("commanded to move down"); 
      if(pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;

       case 's':
      pwmValue = pwmValue + 5;
      driveArdumoto(STOP);
      Serial.println("commanded to move stop"); 
      if(pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;


      
    }
 }
}
