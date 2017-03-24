//#include <SoftwareSerial.h>
#include <stdlib.h>
const int r_arr[]={22,24,26,28,30,32,34,36};
const int channels=8;
const double sensorMin = 1023;       
const double sensorMax = 0;  

String buffer="";
const int analogInPin = A0;
double analog,output;

double mVperAmp = 185; 
double raw= 0;
double offset = 2500; 
double volts = 0;
double amps = 0;

double Min = 0;       
double Max = 0;  
double avgN=20;

boolean inputDone=false;


//SoftwareSerial mySerial(0, 1); // RX, TX
void setup()
{
  buffer.reserve(200);
  analogReference(EXTERNAL);
  for(int i=0;i<channels;i++)
  {
    pinMode(r_arr[i],OUTPUT);
  }//End for loop 
  
  for(int i=0;i<channels;i++)
  {
    digitalWrite(r_arr[i],LOW);
  }//End for loop  

  Serial.begin(9600);
  Serial.println("Im Here!");
    
}//End Relay and serial setup


void loop()
{
  if(inputDone)
  {
   //Serial.println("Success"); 
   String resp=processCommand(buffer);
   delay(100);
   Serial.println(resp); 
   buffer="";
   inputDone=false;
  }
}//End main

void turnOnBoard(int r,int on)
{
  if(r<=8)
  {
    digitalWrite(r_arr[r-1],!on);
  }//End
}//End TurnOn

void serialEvent() 
{
  while (Serial.available()) 
  {
    char in = (char)Serial.read();
        buffer += in;
    if (in == '\n') {
      inputDone = true;
    }//End check for terminator
  }//End serial while loop
}//

 String processCommand(String buffer){
  String respond="";
  if(buffer.charAt(0)=='P')
  { 
    int chan=buffer.charAt(1)-'0';
    int onOff=buffer.charAt(3)-'0';
    turnOnBoard(chan,onOff); 
    String s1=String(chan);
    String s2=String(getCurrent(chan-1));
    respond=String(s1+":"+s2);
  }else if(buffer.charAt(0)=='R')
  {
    int chan=buffer.charAt(1)-'0';
    String s1=String(chan);
    String s2=String(getCurrent(chan-1));
    respond=String(s1+":"+s2);
  }
  return respond;
}//End Process Command

long getCurrent(int chan)
{
    double avg=0;
    Min=sensorMin;
    Max=sensorMax;
    for(int x=0;x<avgN;x++)
    {
      raw=analogRead(chan);
      avg+=raw;
      if(raw>Max){
        Max=raw;}
      if(raw<Min){
       Min=raw;}
      delay(10);
    }//End avg loop
    avg=avg/avgN;
    volts=(avg/1023)*5000;
    amps=((volts-offset)/mVperAmp);
    //debug(avg,Min,Max,volts,amps);
    return (long)(amps*1000); 
}//

void debug(double avg,double minS,double maxS,double v,double a)
{
      Serial.print("Raw: ");
      Serial.print(avg);
      Serial.println("");
      
      Serial.print("Min: ");
      Serial.print(minS);
      Serial.println("");
      
      Serial.print("Max: ");
      Serial.print(maxS);
      Serial.println("");
      
      Serial.print("mV: ");
      Serial.print(v,3);
      Serial.println("");
      
      Serial.print("Amps: ");
      Serial.print(a,3);
      Serial.println("");
      
      Serial.print("per Device: ");
      Serial.print((amps-0.312)/23.0,3);
      Serial.println("");
      
      //Serial.println(analog);
      //Serial.println(output); 
}//
