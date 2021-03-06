/*
 PID  // Here i am only using proportional part
Error is 90 initially i.e. degree required to turn, so error = degree
Motor initial speed 255 and base speed 165 i.e. 255-90 therefore kp = 1
after error is decreased by 5 degree motor speed falls by 5 degree
Now let's implement it

*/
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

MPU6050 mpu;

//mpu vars
#define LED_PIN 13
bool blinkState = false;
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
int runno = 1;
//my vars
const int enable_r=3;
#define echo3 A3 //front forward
#define echo1 A0 // front left
#define echo_r A1
#define echo2 A2 // front right
const int trig1 = 4; //front left
const int trig2 = 13; //front right
const int trig3 = 5; // front forward
const int trig_r = 6;
const int echo_l = 7;
const int trig_l = 8;
const int motor_r1 = 9;//red
const int motor_r2 = 10;//black
const int motor_l1 = 11;//red
const int motor_l2 = 12;//black
const int front_distance = 20;
const int turnradius = 40;
bool rightturn =false, leftturn = false;
int turn = 1;
//pid vars
int error;
int motorbase = 165;

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
float reading(){
  float reading;
 
  mpu.resetFIFO();
  // if programming failed, don't try to do anything
   

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {
        // other program behavior stuff here
        // .
        // .
        // .
        // if you are really paranoid you can frequently test in between other
        // stuff to see if mpuInterrupt is true, and if so, "break;" from the
        // while() loop to immediately process the MPU data
        // .
        // .
        // .
    }

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
        // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetEuler(euler, &q);
            mpu.resetFIFO();
            return euler[0]*180/M_PI;
            
  }

}
long us_reading(int echo_pin, int trig_pin){
  long duration,distance;
  digitalWrite(trig_pin,LOW);
  delayMicroseconds(2);

  digitalWrite(trig_pin,HIGH);
  delayMicroseconds(10);

  digitalWrite(trig_pin,LOW);
  duration = pulseIn(echo_pin,HIGH);

  distance = duration/58.2;
  
  return distance;
}


void turn_right(float read_start,int degree,int voltage){
    float read_final = read_start;
    error = degree;
    if(read_start <(180 - degree) ){ //read_start <110
      while(abs(read_final - read_start)<=degree){
          voltage = motorbase + (error);
          error = degree - (int)abs(read_final - read_start);
          analogWrite(enable_r, voltage);
          analogWrite(enable_r, voltage);
          digitalWrite(motor_r1,HIGH);
          digitalWrite(motor_r2,LOW);
          digitalWrite(motor_l1,HIGH);
          digitalWrite(motor_l2,LOW);
          //if(error % 5 == 0) voltage -= 5;
          read_final=reading();
          Serial.println(read_final);
     }
        
    }
    else if(read_start > (180 - degree)){
      float read_final;
      read_final = read_start;
      float angle1 = abs(180- read_start);
      while(true){
        if(read_final >(180 - degree)){
          voltage = motorbase + error;
          error = degree - (int)(read_final - read_start);
          Serial.println(read_final);
          analogWrite(enable_r, voltage);
          analogWrite(enable_r, voltage);
          digitalWrite(motor_r1,HIGH);
          digitalWrite(motor_r2,LOW);
          digitalWrite(motor_l1,HIGH);
          digitalWrite(motor_l2,LOW);
          //if(error % 5 == 0) voltage -=5;
        }
        else if((angle1 + abs(180 - abs(read_final)))<degree){
          voltage = motorbase + error;
          error = degree - (int)(angle1 + abs(180 - abs(read_final)));
          analogWrite(enable_r, voltage);
          analogWrite(enable_r, voltage);
          digitalWrite(motor_r1,HIGH);
          digitalWrite(motor_r2,LOW);
          digitalWrite(motor_l1,HIGH);
          digitalWrite(motor_l2,LOW);
          Serial.println(read_final);
        }
        else break;
        read_final=reading();
      }
            
    }
    analogWrite(enable_r, 0);
    analogWrite(enable_r, 0);
    
    
}
void turn_left(float read_start,int degree,int voltage){
    float read_final = read_start;
    int error = degree;
    if(read_start >(degree - 180)){
      while(abs(read_start - read_final) < degree ){
       voltage = motorbase + error;
       error = abs(read_start - read_final);
       analogWrite(enable_r, voltage);
       analogWrite(enable_r, voltage);
       digitalWrite(motor_r1,LOW);
       digitalWrite(motor_r2,HIGH);
       digitalWrite(motor_l1,LOW);
       digitalWrite(motor_l2,HIGH);
       Serial.println(read_final);
       read_final = reading();
      }
    }
    else if(read_start < (degree - 180)){
      float read_final;
      read_final = read_start;
      float angle1 = abs(180.0- abs(read_start));
      while(true){
        voltage = motorbase + error;
        if(read_final < (degree - 180)){
          error = degree - (180 - abs(read_final));
          Serial.println(read_final);
          analogWrite(enable_r, voltage);
          analogWrite(enable_r, voltage);
          digitalWrite(motor_r1,LOW);
          digitalWrite(motor_r2,HIGH);
          digitalWrite(motor_l1,LOW);
          digitalWrite(motor_l2,HIGH);
        }
        else if((angle1 + abs(180.0- abs(read_final)))<(degree)){
          error = degree - (int)((angle1 + abs(180.0- abs(read_final))));
          Serial.println(read_final);
          analogWrite(enable_r, voltage);
          analogWrite(enable_r, voltage);
          digitalWrite(motor_r1,LOW);
          digitalWrite(motor_r2,HIGH);
          digitalWrite(motor_l1,LOW);
          digitalWrite(motor_l2,HIGH);
        }
        else break;
        read_final = reading();
      }
      
    }
}
void movebot()
{ 
  analogWrite(enable_r, 255);
  analogWrite(enable_r, 255); 
  digitalWrite(motor_r1,LOW);
  digitalWrite(motor_r2,HIGH);
  digitalWrite(motor_l1,HIGH);
  digitalWrite(motor_l2,LOW);
  Serial.println("DELAY STARTED");
  delay(500);
  Serial.println("DELAY STOPPED");
  
  
}
void turn_rightabout()
{
  Serial.println("enter");
  float temp_dump1;  
  for(int i=0;i<3;i++){
    temp_dump1 = reading();
    Serial.print(temp_dump1);
    Serial.println(" Dummy reading");
  }  
  turn_right(reading(),85,255);
  //turn_right(reading(),25,170);
  
  Serial.println("exit");

    
    
  movebot();
 
  Serial.println("enter");
  float temp_dump2;  
  for(int i=0;i<3;i++){
    temp_dump2 = reading();
    Serial.print(temp_dump2);
    Serial.println(" Dummy reading");
  }
  turn_right(reading(),85,255);
  //turn_right(reading(),25,170);
  Serial.println("exit");
  
  
  return;
}
void turn_leftabout()
{
  Serial.println("enter");
  float temp_dump1;  
  for(int i=0;i<3;i++){
    temp_dump1 = reading();
    Serial.print(temp_dump1);
    Serial.println(" Dummy reading");
  }  
  turn_left(reading(),90,255);
  //turn_left(reading(),25,170);
  
  Serial.println("exit");
    
  movebot();
 
  Serial.println("enter");
  float temp_dump2;  
  for(int i=0;i<3;i++){
    temp_dump2 = reading();
    Serial.print(temp_dump2);
    Serial.println(" Dummy reading");
  }
  turn_left(reading(),90,255);
  //turn_left(reading(),25,170);
  Serial.println("exit");
  
  
  return;
}




void dmpDataReady() {
    mpuInterrupt = true;
}




void setup() {
  // put your setup code here, to run once:
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    
    Serial.begin(115200);
    

    

    // initialize device
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // wait for ready
    /*Serial.println(F("\nSend any character to begin DMP programming and demo: "));
    while (Serial.available() && Serial.read()); // empty buffer
    while (!Serial.available());                 // wait for data
    while (Serial.available() && Serial.read()); // empty buffer again
    */
    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
    
  pinMode(echo1,INPUT);
  pinMode(echo_r,INPUT);
  pinMode(echo_l,INPUT);
  pinMode(echo2,INPUT);
  pinMode(trig1,OUTPUT);
  pinMode(trig2,OUTPUT);
  pinMode(trig_r,OUTPUT);
  pinMode(trig_l,OUTPUT);
  pinMode(enable_r,OUTPUT);
  pinMode(enable_r,OUTPUT);
  pinMode(motor_r1,OUTPUT);
  pinMode(motor_r2,OUTPUT);
  pinMode(motor_l1,OUTPUT);
  pinMode(motor_l2,OUTPUT);

}


void algorithm(){
  long usf1 = us_reading(echo1, trig1);
  long usf2 = us_reading(echo2, trig2);
  long usr = us_reading(echo_r,trig_r);
  long usl = us_reading(echo_l,trig_l);
  Serial.println(usr);
  if((usf1>=front_distance) && (usf2>=front_distance) ){
      //Move Forward
      digitalWrite(enable_r, HIGH);
      digitalWrite(enable_r, HIGH);
      digitalWrite(motor_r1,LOW);
      digitalWrite(motor_r2,HIGH);
      digitalWrite(motor_l1,HIGH);
      digitalWrite(motor_l2,LOW);
      //  delay(50);//move forward for 50ms to avoid excessive load
    }
    else if((usf1<=front_distance) || (usf2<=front_distance)){
      if(usr>=turnradius && usl<=turnradius){
        //Turn Right 
        //take mpu read and pass to turn
        
        turn_rightabout();
        //Store the current turn value
        turn++;  
        rightturn=true;
        leftturn=false;      
      }
      else if(usr<=turnradius && usl>=turnradius){
        //Turn left
        //take mpu read and pass to turn
        turn_leftabout();
        turn++;
        //Store the current turn value   
        rightturn=false;
        leftturn=true;     
      }
      else if(usr<=turnradius && usl<=turnradius){
        //Stop the bot by making both the enable pins low
        analogWrite(enable_r, 0);
        analogWrite(enable_r, 0);
      }
      else if(usr>=turnradius && usl>=turnradius){
        //Turn according to feedback!!!
        if(leftturn && !rightturn){
          //Take right about turn
          //take mpu read and pass to turn
          turn_rightabout();
          turn++;
          rightturn=true;
          leftturn=false; 
          
        }else{
          //Take left about turn
          //take mpu read and pass to turn
          turn_leftabout();
          turn++;
          rightturn=false;
          leftturn=true; 
        }
      }
    }
    /*else if ((usf1>=front_distance) && (usf2>=front_distance) && (us_reading(echo3, trig3)>=front_distance)){
      //move backwards
      digitalWrite(enable_r, HIGH);
      digitalWrite(enable_r, HIGH);
      digitalWrite(motor_r1,HIGH);
      digitalWrite(motor_r2,LOW);
      digitalWrite(motor_l1,LOW);
      digitalWrite(motor_l2,HIGH);
      delay(1000);
      if(leftturn && !rightturn){
          //Take right about turn
          //take mpu read and pass to turn
          turn_rightabout();
          turn++;
          rightturn=true;
          leftturn=false; 
          
        }else{
          //Take left about turn
          //take mpu read and pass to turn
          turn_leftabout();
          turn++;
          rightturn=false;
          leftturn=true; 
        }*/
      

    
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!dmpReady) return;
  if(runno == 1){
    delay(20000);
    runno++;
    
    for(int i=1;i<10;i++){
      float temp;
      temp = reading();
    }
     
  }

  //long us = us_reading(echo1,trig1);
  if(turn%15 == 0){
      setup();  
      algorithm();
      if (turn%15 == 0) turn = 1;
  }else{
      algorithm();      
  }
}

  
 

