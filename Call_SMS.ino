#include "SIM900.h"
#include <SoftwareSerial.h>
#include "call.h"
#include "sms.h"

#define ACTIVE LOW//for buttons

const int ledPinC = 13;      //number of the LED pin
const int ledPinS = 12;

const int buttonPinC = 7;    //number of the Button pin
const int buttonPinS = 4;

//Call
CallGSM call;
boolean started=false;
int buttonStateC = 1;    
boolean calling = false;
boolean caller = false;

//SMS
SMSGSM sms;
int buttonStateS = 1;
int numdata;
char smsbuffer[160];
char n[20];
char sms_position;
char phone_number[20];
char sms_text[100];
int i;
boolean listener=false;
boolean sender=false;

void setup(){
  
    Serial.begin(9600);
    pinMode(ledPinC,OUTPUT);
    pinMode(ledPinS,OUTPUT);
    
    pinMode(buttonPinC, INPUT);
    pinMode(buttonPinS, INPUT);
    
    //when a button is pressed voltage goes to ground (Input reads LOW)
    digitalWrite(buttonPinC,HIGH);
    digitalWrite(buttonPinS,HIGH);
    
    if (gsm.begin(9600)) 
    {
        Serial.println("\nstatus=READY");
        started=true;   
        blink(4,'b');
    } 
    else 
        Serial.println("\nstatus=IDLE");
}

void loop(){
    checkButtons();
    
    //----------------------
    
    //listen for sms and incoming calls
    //when both buttons are pressed at the same time
    if(buttonStateC == ACTIVE && buttonStateS == ACTIVE){
      delay(500);
      checkButtons();
      if(buttonStateC == ACTIVE && buttonStateS == ACTIVE){
        blink(10,'b');
        if(!listener){
        Serial.println("Listener Mode...");
        }else if(listener){
        Serial.println("Exiting Listener Mode...");
        }
        listener = !listener;    //switch
      }  
    }
    
    //call prefixed phone number
    //when the yellow button is pressed
    if(buttonStateC == ACTIVE && buttonStateS != ACTIVE){
      delay(400);
      checkButtons();
      if(buttonStateC == ACTIVE && buttonStateS !=ACTIVE){
        if(!calling){
          Serial.println("Caller Mode...");
        }else if(calling){
          Serial.println("Exiting Caller Mode...");      
        }
        caller = true;    
      }  
    }
    
    //send sms to a prefixed number 
    //when the red button is pressed.
    if(buttonStateS == ACTIVE && buttonStateC != ACTIVE){
      delay(400);
      checkButtons();
      if(buttonStateS == ACTIVE && buttonStateC == HIGH){
        Serial.println("SMS Mode...");
        sender = true;    
      }  
    }
    //-------------------------
    
    
    
    
    
    
/////////    
    if (caller && started) {//call
     /*3 cases for yellow button
      CALL_NONE         - no call activity
      CALL_INCOM_VOICE  - incoming voice
      CALL_ACTIVE_VOICE - active voice
      CALL_NO_RESPONSE  - no response to the AT command
      CALL_COMM_LINE_BUSY - comm line is not free*/
       if(calling)
       {
          digitalWrite(ledPinC,LOW);
          calling = false;
          Serial.println("\nHanging up...\n"); 
          call.HangUp();
          delay(1000);
       }else if(calling==false && call.CallStatus() == CALL_NONE)
       {
          calling = true;
          digitalWrite(ledPinC, HIGH);
          delay(1000);
          Serial.println("\nCalling...\n");
          call.Call("INSERT_PHONE_NUMBER_HERE"); 
          
       }else if(calling==false && call.CallStatus() == CALL_INCOM_VOICE){
          calling = true;
          digitalWrite(ledPinC, HIGH);
          delay(1000);
          Serial.println("\nSpeaking...\n");
          call.PickUp();
       
       }
       caller=false;
    }
//////////
    if (sender && started) { //sms
       digitalWrite(ledPinS, HIGH);
       delay(3000);
       if (sms.SendSMS("INSERT_PHONE_NUMBER_HERE", "Arduino Phone SMS Message")){
          blink(3,'r');
          Serial.println("\nSMS sent OK.");
        }
        else
        {
          digitalWrite(ledPinS, LOW);
          Serial.println("\nError sending SMS.");
        }      
        sender=false;
    }
///////////    
    if(listener && started){ //listen
          //Read if there are messages on SIM card and print them.
          sms_position=sms.IsSMSPresent(SMS_UNREAD);
          if (sms_position) {
            // read new SMS
            Serial.print("SMS postion:");
            Serial.println(sms_position,DEC);
            sms.GetSMS(sms_position, phone_number, sms_text, 100);
            // now we have phone number string in phone_num
            Serial.println(phone_number);
            // and SMS text in sms_text
            Serial.println(sms_text);
          }   
          else{
            Serial.println("NO NEW SMS,WAITTING");
          }
     
         //check for incoming calls
          if(call.CallStatus() == CALL_INCOM_VOICE){
            Serial.println("INCOMING CALL!");
            blink(12,'b');
          }else{
            Serial.println("NO NEW CALL, WAITING");
          }
     
          delay(2000); 
    }


}





//blink function
void blink(int rep,char x){//blinks LEDS
  if(x=='y'){//blink yellow
     for(int i=0;i<rep;i++){
          digitalWrite(ledPinC, HIGH);
          delay(100);
          digitalWrite(ledPinC, LOW);
          delay(100);
     }
  }else if(x=='r'){//blink red
     for(int i=0;i<rep;i++){
          digitalWrite(ledPinS, HIGH);
          delay(100);
          digitalWrite(ledPinS, LOW);
          delay(100);
     }
  }else{//blink both
     for(int i=0;i<rep;i++){
          digitalWrite(ledPinC, HIGH);     
          digitalWrite(ledPinS, HIGH);
          delay(100);
          digitalWrite(ledPinC, LOW);
          digitalWrite(ledPinS, LOW);
          delay(100);
     }  
  }
}

//refreshes button states
void checkButtons(){
    buttonStateC = digitalRead(buttonPinC);
    buttonStateS = digitalRead(buttonPinS);
}
