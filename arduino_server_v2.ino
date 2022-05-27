
/* 
 * purpose: This program acts as an ethernet-server. The server waits 
 * for the first available client and initiates a connection. 
 * It reads the value of a fotoresistor and sends this 
 * information to the client. The server receives information 
 * from the client. The information determines what state the 
 * program is in.
 * 
 * authors: Hedda Eriksson, Malin Ramkull
 * 
 * version: 1.0
 * 
 * date: tue 10 may  
 * 
 * fotovalue:
 * This integer is a value between 0-1023. It varies depending
 * on the luminosoty in the room. If the value is lower than 
 * 1000 the sensor is considered to be "covered". If the value 
 * is equal or greater than 1014 the sensor is not covered.
 * 
 * messageToServer:
 * This char holds information that is to be sent to the client.
 * 'F' means that the sensor is not covered.
 * 'T' means that the sensor is covered. 
 * 
 * messageFromServer:
 * This char holds information that is received from the client. 
 * 'a' means that music is playing.
 * 'b' means that music is paused.
 *
 */
 
#include <Ethernet.h>
#include <SPI.h>
#include <Servo.h>

EthernetServer server(1337);
Servo servo;

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x59, 0x67 }; 
byte ip[] = { 192, 168, 0, 12 };                      

const int fotopin = A0;
const int laserpin = 2;
const int motorpin = 9;
const int ethernetpin = 13;

int state = 5;
int fotovalue = 0;

char messageToServer;
char commandFromServer;

void setup() {
  pinMode(ethernetpin,OUTPUT);
  pinMode(laserpin,OUTPUT);
  pinMode(fotopin, INPUT);
  pinMode(motorpin, OUTPUT);
  delay(100);

  servo.attach(motorpin);
  servo.write(0);
  
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  delay(10);
}

void loop() {
  EthernetClient client = server.accept(); 
  delay(100);

  while(client.connected()){
    fotovalue = analogRead(fotopin);
    delay(10);
    commandFromServer = client.read();
    delay(10);

    switch(state){
      
      case 1:                         //validate the fotovalue
        digitalWrite(laserpin,LOW);
        delay(10);
        if(fotovalue >= 1000){
          messageToServer = 'F';
          state = 5;
          servo.write(180);
          delay(760);
        }
        if(fotovalue < 1000){
          messageToServer = 'T';
          if(commandFromServer == 'a'){ 
             state = 1;
         }
         if(commandFromServer == 'b'){
             state = 2;
         } 
      } 
      break;
      
      case 2:                        //turn servo motor 0-180°
        servo.write(180);
        delay(760);
        state = 3;
      break;
      
      case 3:                       //turn on laser
        digitalWrite(laserpin,HIGH);
        delay(10);   
        if(fotovalue >= 1000){
          messageToServer = 'F';
          state = 5;
        }
        if(fotovalue < 1000){
          messageToServer = 'T';
          if(commandFromServer == 'a'){
            state = 4;
          }
          if(commandFromServer == 'b'){
            
            state = 3;
          }
        }
      break;

      case 4:                       //turn servo motor 180-0°
        servo.write(0);
        delay(10);
        state = 1;
      break;

      case 5:                       //wait for sensor to be covered
      digitalWrite(laserpin,HIGH);
      if(fotovalue >= 1000){
        messageToServer = 'F';
        state = 5;
      }
      if(fotovalue < 1000){
        messageToServer = 'T';
        state = 1;
        servo.write(0);
      }
      break;
    }
    delay(10);
    client.write(messageToServer);
    delay(10);
    Serial.println(fotovalue);
  }
}
