
#include "arduino_secrets.h"

////////////////////////////////////////////// 
//        RemoteXY include library          // 
////////////////////////////////////////////// 

// RemoteXY select connection mode and include library  
#define REMOTEXY_MODE__ESP8266WIFI_LIB_CLOUD
#include <ESP8266WiFi.h> 

#include <RemoteXY.h> 

// RemoteXY connection settings  
#define REMOTEXY_WIFI_SSID SECRET_SSID 
#define REMOTEXY_WIFI_PASSWORD SECRET_PASS 
#define REMOTEXY_CLOUD_SERVER "cloud.remotexy.com" 
#define REMOTEXY_CLOUD_PORT 6376 
#define REMOTEXY_CLOUD_TOKEN SECRET_CLOUD_TOKEN 

// RemoteXY configurate   
#pragma pack(push, 1) 
uint8_t RemoteXY_CONF[] = 
  { 255,2,0,0,0,19,0,13,13,0,
  4,48,15,7,15,46,2,26,4,176,
  48,23,46,15,2,26 }; 
   
// this structure defines all the variables and events of your control interface  
struct { 

    // input variables
  int8_t slider_1; // =-100..100 slider position 
  int8_t slider_2; // =-100..100 slider position 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY; 
#pragma pack(pop) 


///////////////////////////////////////////// 
//           END RemoteXY include          // 
///////////////////////////////////////////// 

#include <Servo.h>
#include <NoDelay.h>

//L9110S motor drive input pin
#define IN_1  14           // L9110S B-2A motors Right       GPIO14(D5)
#define IN_2  4            // L9110S B-1A motors Right       GPIO4(D4)
#define IN_3  13           // L9110S A-1B motors Left        GPIO13(D7)
#define IN_4  12           // L9110S A-1A motors Left        GPIO12(D6)
#define ServoPin  0        // ServoPin Input pin             GPIO0(D8)

// Define the ultrasonic sensor pin
#define  Trig 16    //GPIO16(D2)
#define  Echo 5     //GPIO5(D3)

noDelay debugTime(1000);
noDelay remoteCommandTime(30);
noDelay collisionCheckTime(100);

int speedCar = 150;         // 0 - 255.
int speedSteering = 90;
int speedAdjustment = -10;

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

void setup()  
{ 
  Serial.begin(115200);

  //Set the pin mode
  pinMode(IN_1, OUTPUT);
  pinMode(IN_2, OUTPUT);
  pinMode(IN_3, OUTPUT);
  pinMode(IN_4, OUTPUT); 
  pinMode(Trig,OUTPUT);
  pinMode(Echo,INPUT);

  RemoteXY_Init ();  
   
   
  // attaches the servo on GPIO0 to the servo object
  myservo.attach(ServoPin);  
  myservo.write(100);  
  analogWriteFreq(512);
   
} 

void loop()  
{  
  RemoteXY_Handler (); 
   

   if(debugTime.update()) {
    Serial.print("Hastighet: ");
    Serial.print(RemoteXY.slider_1);
    Serial.print(", Styrning: ");
    Serial.println(RemoteXY.slider_2);
   }

  if(remoteCommandTime.update()) {
     if (RemoteXY.slider_1>0) {
       goAhead();
     }else if(RemoteXY.slider_1<0) {
       goBack();
     }
  
     if (RemoteXY.slider_2>0) {
       goRight();
     }else if(RemoteXY.slider_2<0) {
       goLeft();
     }
  
  }
  if(collisionCheckTime.update()) {
    int distance = GetDistance();
    if(distance<20) {
      goBack();
    }else if(RemoteXY.slider_1==0 && RemoteXY.slider_2==0) {
      stopRobot();
    }
  }
   if (RemoteXY.connect_flag == 0) {
    stopRobot();
   }

}

void goAhead(){ 

      analogWrite(IN_1, 0);
      analogWrite(IN_2, speedCar);
      
      analogWrite(IN_3, 0);
      analogWrite(IN_4, speedCar + speedAdjustment);
}

void goBack(){ 
      analogWrite(IN_1, speedCar);
      analogWrite(IN_2, 0);
     
      analogWrite(IN_3, speedCar + speedAdjustment);
      analogWrite(IN_4, 0);   
}

void goRight(){ 

      analogWrite(IN_1, speedSteering);
      analogWrite(IN_2, 0);
      
      analogWrite(IN_3, 0);
      analogWrite(IN_4, speedSteering);  
}

void goLeft(){

      analogWrite(IN_1, 0);
      analogWrite(IN_2, speedSteering);
     
      analogWrite(IN_3, speedSteering);
      analogWrite(IN_4, 0);   
}

void stopRobot(){  

      digitalWrite(IN_1, LOW);
      digitalWrite(IN_2, LOW);
      digitalWrite(IN_3, LOW);
      digitalWrite(IN_4, LOW);  
   
}

/*
Function: obtain ultrasonic sensor ranging data
Parameter description: sensor is connected to the motherboard pin port A1,A2
*/
float GetDistance()
{
  float distance;
  
  digitalWrite(Trig, LOW); 
  delayMicroseconds(2); 
  digitalWrite(Trig, HIGH); 
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  
  distance = pulseIn(Echo, HIGH) / 58.00;
  Serial.print("distance = ");
  Serial.println(distance);
  
  return distance;
}


/*
* Function: Obstacle avoidance
* Parameter: set_dis sets the obstacle avoidance distance
*/
void avoidance(int set_dis)
{
    int angle;
    int dis[3];// distance
  
     goAhead();
     myservo.write(100); //Steering engine back to center
    //Obtain the distance between the cart and the obstacle and store it in dis[1]
    dis[1] = GetDistance(); 

    if(dis[1] < set_dis )
    {
        stopRobot();  //Stop the car

        //If the left and right infrared obstacle avoidance sensors do not encounter obstacles when the steering gear rotates
        
            for (angle = 100; angle <= 180; angle++) 
            {
                 myservo.write(angle);
                //delay(1);         
            }
             delay(5); 
           // Measure the distance between the left obstacle and the cart, and store the measurement data in dis[2]
            dis[2]=GetDistance(); 

            for (angle = 100; angle >= 0; angle--) 
            {
                 myservo.write(angle);  
                          
                if(angle == 100)  
                {   // Stores measurement data in dis[1]
                      delay(5); 
                    dis[1] = GetDistance(); 
                } 
            }
           // Record the range data on the right side of the trolley
            dis[0] = GetDistance(); 
            delay(5);
           
            for (angle = 0; angle <= 100; angle++) 
            {
                 myservo.write(angle);
            }
        //The right is more distant from the obstacle than the Left
        if(dis[0] < dis[2] ) 
        {
          if(dis[0] < 10)
          {
            goBack();
            delay(300);
          }
            goLeft();
            delay(150);
        }
        //The right is more distant from the obstacle than the Right
        else if (dis[0] > dis[2] )
        {
             if(dis[0] < 10)
            {
              goBack();
              delay(300);
            }
              
            goRight(); 
            delay(150);
        } 
    }
}
