/*........................  Firmware  System for Emergency Lung Ventilator v 0.0.  2020/05/23  Set-up: 4 STEPPER Motors & 2 BT6600 drivers / 2 limitswitchers / 1 I2C 16x2 Monochromatic display / 3x analog potenciometer / 2x standard switches  Written by: DEX Innovation Centre*//*  Current version includes:  - Homing of both Arms, but not separately   - Pressing bag by very basic code, without any adjustments to the real inputs  - I2C Monochramic display integrated  - Analog potentiometer control integrated  - Set-up of operation values including visualisation at Display  - Controls of the System, basic functionality (depends on the readiness of the prototype)  - READY TO SET-UP FOR PRESSIRNG REAL TIDAL VOLUME    - Version was tested at the prototype NO.2*//* ADDED FROM LAST RELEASE- 2 Arms management - independent homing of both arms - DONE & TESTED- 2nd version of the potentiometers (664 max) - DONE & TESTED- control of the system movements - calibrating during the main loop, if limit switch is pressed - DONE& alpha tested *//*Next version: - Add the Homing message to the Display- Control of Respiratory Rate and of the I/E ( to be still included to 0.0.4 version)- Fundamnetal redesign thanks to reading of one switcher ( possibility to change the values and home during the operation) */#include <AccelStepper.h>#include <Wire.h>#include <LiquidCrystal_I2C.h>/*DEFINE THE PIN'S WIRING FOR THE WHOLE SYSTEM  DIR for BT6600 drivers (connected in series) - PIN 2  STEP for BT6600 drivers (connected in series) - PIN 3  LEFT LIMIT SWITCH (looking from the head of Bag - head is the side connected to lung/patient) - PIN 8  RIGHT LIMIT SWITCH (looking from the head of Bag - head is the side connected to lung/patient) - PIN 9  I2C Display SCL connection - SCL PIN   I2C Display SDA connection - SDA PIN   Left / AirVolume Potentiometer - A1 PIN  Middle / RRValue Potentiometer - A2 PIN  Rigth / IERatio Potentiometer - A3 PIN   */  #define Right_DirPin 2#define Right_StepPin 3#define Left_DirPin 4#define Left_StepPin 5#define Left_Limit_Switch 7#define Right_Limit_Switch 8#define AirVolume_PIN  A1#define RRValue_PIN A2#define IERatio_PIN A3// Create a new instance of the AccelStepper class:AccelStepper Right_CAMArms = AccelStepper(1, Right_StepPin, Right_DirPin);AccelStepper Left_CAMArms = AccelStepper(1, Left_StepPin, Left_DirPin);//Arms failure valuesint Left_Arm_Failure;int Right_Arm_Failure; //Create a new instance for MonoChromatic I2C DisplayLiquidCrystal_I2C lcd(0x27, 16, 2);//definition of supportive variableslong Long_i = 1; // Used to: i) homing Right_CAMArms ; ii) Supportive variable to run the definition loop for the inputs// global variables defining key parameters for the ventilationint TidalVolume; // Tidal Volume (TV) (air volume pushed into lung): between 200 – 800 mL based on patient weight.int RRValue; // Respiratory Rate (RR) (breaths per minute): between 6 – 40. Note that the low RRs of 6 – 9 are only applicable to Assist Control.float IERatio; // (inspiratory/expiration time ratio): recommended to start around 1:2; best if adjustable between range of 1:1 – 1:3*./*-------------------------FOR TESTING PURPOSES ONLY --------------------------------------------------*/unsigned long time;void setup() {  // just for testing  Serial.begin(9600);   // just for testing     //Initialization screen for the DEX IC VENT  lcd.init();  lcd.begin(16,2);  lcd.backlight();    lcd.setCursor(3,0);   lcd.print("DEX-IC VENT");   lcd.setCursor(5,1);  lcd.print("Welcome");  delay(3000);  lcd.clear();    //set-up of the text, which will stay on the screen permanenty  lcd.setCursor (2,0);  lcd.print("T. Volume ");  lcd.setCursor (0,1);  lcd.print("RR ");  lcd.setCursor (7,1);  lcd.print("I/E 1: ");      // just for testing  time=0;  // just for testing      //start of the reading of the inputs from the Potentiometers  while (time<20000) /// BE AWARE FOR CURRENT VERSION THERE IS 20.000 of miliseconds to set-up the system, then it starts automatically. If you need to shorten or prolong, please change the variable here   {    // reading of the Tidal (Air) Volume, while linear conversion rate is applied. The limits: 200 - 800 ml    lcd.setCursor(13,0);    int A1Reading = analogRead(AirVolume_PIN);    //!!! VERSION FOR PROTOTYPE NO.2    //TidalVolume=200+A1Reading/1.705; // HERE CHANGE IF YOU NEED TO CHANGE THE RATIO    //!!! VERSION FOR THE PROTOTYPE NO.3    TidalVolume=200+A1Reading/1.105; // HERE CHANGE IF YOU NEED TO CHANGE THE RATIO    lcd.print(TidalVolume);        //reading of the RR (Respiratory Rate), while linear conversion rate is applied. The limits: 6 - 40 RR per minute     lcd.setCursor(3,1);        int A2Reading = analogRead(RRValue_PIN); // HERE CHANGE IF YOU NEED TO CHANGE THE RATIO    //!!! VERSION FOR PROTOTYPE NO.2    //RRValue=6+A2Reading/30;    //!!! VERSION FOR PROTOTYPE NO.3    RRValue=6+A2Reading/19.5;    if (RRValue < 10) {      lcd.print("0");      lcd.print(RRValue);    }    else {      lcd.print(RRValue);     }        // reading of the Expiratory value, while linear conversion rate is applied. The limits of I/E are from 1:1 to 1:3    lcd.setCursor(13,1);    int A3Reading = analogRead(IERatio_PIN);    //!!! VERSION FOR PROTOTYPE NO.2    //IERatio=1.0+A3Reading/510.0; // HERE CHANGE IF YOU NEED TO CHANGE THE RATIO    //!!! VERSION FOR PROTOTYPE NO.3    IERatio=1.0+A3Reading/331.0; // HERE CHANGE IF YOU NEED TO CHANGE THE RATIO    lcd.print(IERatio);           /*  -------------------------  FOR TESTING PURPOSES ONLY   -------------------------  -------------------------  */     time = millis();  }      /*  -------------------------  //HOMING OF THE SYSTEM  -------------------------  -------------------------  */    pinMode(Left_Limit_Switch, INPUT);  pinMode(Right_Limit_Switch, INPUT);  delay(10);  //  Set Max Speed and Acceleration of each Steppers at startup for homing  Right_CAMArms.setMaxSpeed(4000);  Left_CAMArms.setMaxSpeed(4000);  // Homing of both arms   while (!digitalRead(Left_Limit_Switch) && !digitalRead(Right_Limit_Switch)) {  // CAMArm is opening till it hits the switch    Right_CAMArms.moveTo(Long_i);  // Set the position to move to    Left_CAMArms.moveTo(Long_i);    Right_CAMArms.setSpeed(40);  // Set Speed of Stepper (Slower to get better accuracy)    Left_CAMArms.setSpeed(40);    Long_i++;  // Decrease by 1 for next move if needed    Right_CAMArms.run();  // Stepper does 1 step (possibility to test to decrease Right_StepPing to 1/2 step)    Left_CAMArms.run();    delay(5);  }  delay(1000);  // One ARM is in the home position, but not both. Test which one is homed and moved another there :)   if (!digitalRead(Right_Limit_Switch)) {    // RIGHT Cam ARM has to be still homed     while (!digitalRead(Right_Limit_Switch)) {  // CAMArm is opening till it hits the switch      Right_CAMArms.moveTo(Long_i);  // Set the position to move to      Right_CAMArms.setSpeed(40);  // Set Speed of Stepper (Slower to get better accuracy)      Long_i++;  // Decrease by 1 for next move if needed      Right_CAMArms.run();  // Stepper does 1 step (possibility to test to decrease Right_StepPing to 1/2 step)      delay(5);    }  }   else {      // LEFT Cam ARM has to be still homed     while (!digitalRead(Left_Limit_Switch)) {  // CAMArm is opening till it hits the switch      Left_CAMArms.moveTo(Long_i);  // Set the position to move to      Left_CAMArms.setSpeed(40);  // Set Speed of Stepper (Slower to get better accuracy)      Long_i++;  // Decrease by 1 for next move if needed      Left_CAMArms.run();  // Stepper does 1 step (possibility to test to decrease Right_StepPing to 1/2 step)      delay(5);    }  }  delay(2000);  // The both Limit Switches are pressed now  // So we will be opening both ARMS a bit, we asume that both ARMS are opening freely   while (digitalRead(Left_Limit_Switch)) { // Make the Stepper move CW until the switch is deactivated    Right_CAMArms.moveTo(Long_i);    Left_CAMArms.moveTo(Long_i);    Right_CAMArms.setSpeed(-40);  // Set Speed of Stepper (Slower to get better accuracy)    Left_CAMArms.setSpeed(-40);    Right_CAMArms.run();    Left_CAMArms.run();    Long_i--;    delay(5);  }  // despite there is no contact in the circuit, the switchers are mechanically still touched, better to move a bit more away  for (int count = 0; count <60; count++) { // Change CONSTANT 60 to adjust the arms to be just touching limit switch     Right_CAMArms.moveTo(Long_i);    Left_CAMArms.moveTo(Long_i);    Right_CAMArms.setSpeed(-40);  // Set Speed of Stepper (Slower to get better accuracy)    Left_CAMArms.setSpeed(-40);    Right_CAMArms.run();    Left_CAMArms.run();    Long_i--;    delay(5);  }  delay(3000);  // Arms are in HOME POSITION  //--------------------------  Right_CAMArms.setCurrentPosition(0);  Left_CAMArms.setCurrentPosition(0);  delay(3000);  }void loop() {    //Deleting of the possible Failures of both arms:  Left_Arm_Failure = 0;  Right_Arm_Failure = 0;      /* TO BE SET-UP BASED ON THE REAL READINGS FOR THE TIDAL VOLUME   The TidalVolume is integral value which is set in the ration from 200 to 800 ml. The objective is that system flows correct amount of the air volume.   The current function is linear one, while 200 ml was set-up to be equal position -60 and 800 ml was to be equal to - 160  For changes modify the standard linear mapping equation( values 6 and 60)  =(TidalVolume-200)/6+60  How to work with mapping: Tidal Volume is from 200 to 800 (600 values), while ArmPosition is assumed to be from 60 to 160. It means that EACH CHANGE OF 1 FOR ARM MEANS CHANGE   OF 6 IN TIDAL VOLUME. 60 represents the arm position for lowest tidal volume (200 ml), so if it is not true, then redefine it.   */  int ArmPosition = (TidalVolume-200)/6+60;    // This code is CLOSING ARM  //-------------------------  // Position of the arm is already managed by the Tidal Volume function, however the equation is NOT TESTED  while(Right_CAMArms.currentPosition() != -ArmPosition && Left_CAMArms.currentPosition() != -ArmPosition)  {    Right_CAMArms.setSpeed(-250);// this number regulates the speed of closing the arms     Right_CAMArms.runSpeed();    Left_CAMArms.setSpeed(-250);// this number regulates the speed of closing the arms     Left_CAMArms.runSpeed();  }  delay(1000);    //THIS CODE IS OPENING ARM  //-------------------------  while(Right_CAMArms.currentPosition()!= 0 && Left_CAMArms.currentPosition() != 0 && Right_Arm_Failure == 0 && Left_Arm_Failure == 0)  //this number shall be reverse to the function while for opening arm   {    Left_CAMArms.setSpeed(250);// this number regulates the speed of closing the arms     Left_CAMArms.runSpeed();    Right_CAMArms.setSpeed(250); // this number regulates the speed of opening the arms     Right_CAMArms.runSpeed();    if (digitalRead(Right_Limit_Switch)) { Right_Arm_Failure = !Right_Arm_Failure;}     if (digitalRead(Left_Limit_Switch)) { Left_Arm_Failure = !Left_Arm_Failure;}      }  //RIGT ARM FAILURE => We will let fail also opposite arm  //------------------------------------------------------  if (Right_Arm_Failure == 1)  {    while (Left_Arm_Failure == 0)    {    Left_CAMArms.setSpeed(250);// this number regulates the speed of closing the arms     Left_CAMArms.runSpeed();    if (digitalRead(Left_Limit_Switch)) { Left_Arm_Failure = !Left_Arm_Failure;}         }  }   //LEFT ARM FAILURE => in any case, we have managed to fail Left Arm too. If Right Arm failure was cause, the code will immediately loop out   //------------------------------------------------------  if (Left_Arm_Failure == 1)  {    while (Right_Arm_Failure == 0)    {    Right_CAMArms.setSpeed(250);// this number regulates the speed of closing the arms     Right_CAMArms.runSpeed();    if (digitalRead(Right_Limit_Switch)) { Right_Arm_Failure = !Right_Arm_Failure;}         }  }    //IN case of any failure we run as quick as possible the homing procedure (note that if ANY FAILURE happened, both Failures (right/left) are already activated)  // IN-CYCLE HOMING PROCEDURE  // -------------------------  if (Right_Arm_Failure == 1)  {    Long_i=0;    // The both Limit Switches are pressed now    // So we will be opening both ARMS a bit, we asume that both ARMS are opening freely     while (digitalRead(Left_Limit_Switch)) { // Make the Stepper move CW until the switch is deactivated      Right_CAMArms.moveTo(Long_i);      Left_CAMArms.moveTo(Long_i);      Right_CAMArms.setSpeed(-950);  // Set Speed of Stepper (Slower to get better accuracy)      Left_CAMArms.setSpeed(-950);      Right_CAMArms.run();      Left_CAMArms.run();      Long_i--;    }    // despite there is no contact in the circuit, the switchers are mechanically still touched, better to move a bit more away    for (int count = 0; count <60; count++) { // Change CONSTANT 60 to adjust the arms to be just touching limit switch       Right_CAMArms.moveTo(Long_i);      Left_CAMArms.moveTo(Long_i);      Right_CAMArms.setSpeed(-950);  // Set Speed of Stepper (Slower to get better accuracy)      Left_CAMArms.setSpeed(-950);      Right_CAMArms.run();      Left_CAMArms.run();      Long_i--;    }  Right_CAMArms.setCurrentPosition(0);  Left_CAMArms.setCurrentPosition(0);  }  delay(1000);}