#include <Servo.h>
#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//define servo motors
Servo s1, s2;

//control(L298N Motor Driver)
int ENA=5, IN1=6, IN2=7; //Left Motor
int ENB=10, IN3=8, IN4=9; //right Motor

//Load Cell
#define DT 14
#define SCK 15
HX711 scale;

//Boundry Detection sensor
#define trig1 16
#define echo1 17
#define trig2 18
#define echo2 19

//Buzer
#define buzzer 4

//voltage Sensor
#define vPin A0


//LCD
LiquidCrystal_I2C lcd(0x27,16,2);

const int spd=255; //Moter Speed
const int stepDelay=15; //delay for removing extra water

long lastWeight=0;

//setup servo motor
void setup(){
  s1.attach(12);
  s2.attach(11);
  moveArm(160);

//ultrasonics setup
  pinMode(ENA,OUTPUT); pinMode(IN1,OUTPUT); pinMode(IN2,OUTPUT);
  pinMode(ENB,OUTPUT); pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);

//initialize load cell
  scale.begin(DT,SCK);

  pinMode(trig1,OUTPUT); pinMode(echo1,INPUT);
  pinMode(trig2,OUTPUT); pinMode(echo2,INPUT);

  pinMode(buzzer,OUTPUT);

//LCD setup
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("System Ready");
  delay(1000);
}

void loop(){
  static bool busy=false;

//read distence
  int d1=busy?999:dist(trig1,echo1); //trash detector
  int d2=busy?999:dist(trig2,echo2); //Boundry check

//trash sensor to 20cm>x
  if(d1>20||d1==0){
    forward();
    show();
  } 
  
  //stop,move little back and collect
  else if(d1<=20&&d1>=10){
    busy=true;
    stopM();
    backward();
    delay(500);
    stopM();
    collect();
    busy=false;
  }

//boundry detect
  if(d2<=35&&d2>=30){
    buzz(2);
    lcd.setCursor(0,0);
    lcd.print("Boundary Detect! ");
    left();
    delay(1000);
    stopM();
  }
}

//arm controling
void moveArm(int angle){
  int c1=s1.read();
  int c2=s2.read();
  if(c1<angle){
    for(int a=c1;a<=angle;a++){
      s1.write(a);
      s2.write(180-a);
      delay(stepDelay);
    }
  } else {
    for(int a=c1;a>=angle;a--){
      s1.write(a);
      s2.write(180-a);
      delay(stepDelay);
    }
  }
}

void forward(){ motor(HIGH,LOW,HIGH,LOW,spd,spd); }
void backward(){ motor(LOW,HIGH,LOW,HIGH,spd,spd); }
void left(){ motor(LOW,HIGH,HIGH,LOW,spd,spd); }
void right(){ motor(HIGH,LOW,LOW,HIGH,spd,spd); }
void stopM(){ motor(LOW,LOW,LOW,LOW,0,0); }

void motor(int i1,int i2,int i3,int i4,int sA,int sB){
  digitalWrite(IN1,i1); digitalWrite(IN2,i2);
  digitalWrite(IN3,i3); digitalWrite(IN4,i4);
  analogWrite(ENA,sA);
  analogWrite(ENB,sB);
}

long dist(int t,int e){
  long r[5];
  for(int i=0;i<5;i++){
    digitalWrite(t,LOW); delayMicroseconds(2);
    digitalWrite(t,HIGH); delayMicroseconds(10);
    digitalWrite(t,LOW);
    long d=pulseIn(e,HIGH,30000)*0.034/2;
    r[i]=(d>0&&d<400)?d:999;
    delay(2);
  }
  for(int i=0;i<4;i++){
    for(int j=i+1;j<5;j++){
      if(r[j]<r[i]){ long tmp=r[i]; r[i]=r[j]; r[j]=tmp; }
    }
  }
  return r[2];
}

void buzz(int m){
  if(m==1) tone(buzzer,1000,200);
  else if(m==2) tone(buzzer,2000,200);
}

void collect(){
  buzz(1);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Trash Collecting");
  moveArm(180);
  delay(1000);
  moveArm(50);
  delay(1000);
  moveArm(70);
  delay(2000);
  moveArm(0);
  delay(1000);
  moveArm(160);
  delay(500);
}

void show(){
  lcd.setCursor(0,0);
  int val=analogRead(vPin);
  float v=val*(5.0/1023.0);
  lcd.print("V:"); lcd.print(v,2); lcd.print("  ");
  if(millis()-lastWeight>1000){
    lastWeight=millis();
    if(scale.is_ready()){
      long r=scale.get_units(3);
      lcd.setCursor(0,1);
      lcd.print("W:"); lcd.print(r); lcd.print("    ");
    }
  }
}



