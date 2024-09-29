//Librerias IOT ETC
#define BLYNK_TEMPLATE_ID "TMPL2cY8imsI-"
#define BLYNK_TEMPLATE_NAME "LED IOT"
#define BLYNK_AUTH_TOKEN "SKRC3OpZcQv30Cji5M7BRYQIfLq7reEf"

#define BLYNK_PRINT Serial
#define APP_DEBUG

#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp32.h>
#include <UniversalTelegramBot.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include <AccelStepper.h>

DHT dht(4,DHT11);
Servo myservo;

char network[] = "Arduino";
char passwords[] = "123456789";

const int stepsPerRevolution = 2048;  
#define IN1 18
#define IN2 19
#define IN3 22
#define IN4 23
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

#define BOT_TOKEN "7209010749:AAFEerzCzcjEyaaDnUzBhU2MyAfEa5jeZxE"
#define CHAT_ID "1792556978"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

int end;
int DHT;
int IRsensor;
BlynkTimer timer;
int last_pos;
int last;
int mq2Pin = 36;
int MQ2Value;

bool measureDistance=false;
float distanceSum = 0;
int measurementCount = 0;

void setup() {
  Serial.begin(115200);

  pinMode(13,INPUT);
  pinMode(mq2Pin,INPUT);

  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);//RELAYS
  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);

  digitalWrite(25,1);
  digitalWrite(26,1);
  digitalWrite(27,1);
  digitalWrite(32,0);
  digitalWrite(33,1);

  myservo.attach(21);
  myservo.write(180);   //inicializa el servomotor en 0 grados.
  //timer.setInterval(1000L, myTimer);
  //timer.setInterval(2000L, myTimerMQ2);
  Blynk.begin(BLYNK_AUTH_TOKEN, network, passwords);
  dht.begin();

  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(100);

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Permite a client realizar conexiones HTTPS
  
  delay(500);
}


void loop() {
  IRsensor = digitalRead(13);
  Serial.print(IRsensor);

  if (IRsensor == 0) {
    bot.sendMessage(CHAT_ID, "SE HA ABIERTO LA PUERTA DE SU CASA!", "");
  }
  distancia();
  timer.run();
  Blynk.run();
  DHTsensor();
  myTimerMQ2();
  myTimer();
  stepper.runSpeed();
}


BLYNK_WRITE(V0) {//LED EN EL PIN 25
  int reles = param.asInt();
  switch(reles){
    case 1:
      digitalWrite(32,1);//DIFERENTE
      digitalWrite(33,1);
      digitalWrite(25,1);
      digitalWrite(26,1);
      digitalWrite(27,1);
      break;
    case 2:
      digitalWrite(32,0);
      digitalWrite(33,0);
      digitalWrite(25,1);
      digitalWrite(26,1);
      digitalWrite(27,1);
      break;
    case 3:
      digitalWrite(32,0);
      digitalWrite(33,1);
      digitalWrite(25,0);
      digitalWrite(26,1);
      digitalWrite(27,1);
      break;
    case 4:
      digitalWrite(32,0);
      digitalWrite(33,1);
      digitalWrite(25,1);
      digitalWrite(26,0);
      digitalWrite(27,1);
      break;
    case 5:
      digitalWrite(32,0);
      digitalWrite(33,1);
      digitalWrite(25,1);
      digitalWrite(26,1);
      digitalWrite(27,0);
      break;
    default:
      digitalWrite(25,1);
      digitalWrite(26,1);
      digitalWrite(27,1);
      digitalWrite(32,0);
      digitalWrite(33,1);
  }
}


void myTimer(){
  float temp = dht.readTemperature();
  Blynk.virtualWrite(V1,temp);
}

void myTimerMQ2(){
  int MQ2 = analogRead(mq2Pin);
  Serial.print("    Valor del sensor MQ2: ");
  Serial.println(MQ2);
  if(MQ2>750){
    digitalWrite(33,0);
    bot.sendMessage(CHAT_ID, "SE HA DETECTADO UNA FUGA DE GAS EN SU CASA", "");
  }
  Blynk.virtualWrite(V2, MQ2); 
}

BLYNK_WRITE(V3) {
  int servomotor= param.asInt();
  if(servomotor==1 && last_pos!=servomotor ){
    bot.sendMessage(CHAT_ID, "ALIMENTANDO", "");
    myservo.write(0);   
    delay(3000);
    myservo.write(180);
  }
  last_pos=servomotor;
}

BLYNK_WRITE(V4) {
  end = param.asInt();
}


void DHTsensor(){
  float temp = dht.readTemperature();
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C ");
}

float predict(float x) {
    float coefficients[] = {0.00000000e+00, 1.29301789e+01, 1.21904834e+00, 4.56564712e-02, 7.78878780e-04, 4.68732162e-06};
    float intercept =35.42782857 ;

    float y_pred = intercept;
    for (int i = 0; i < 6; i++) {
        y_pred += coefficients[i] * pow(x, i);
    }

    return y_pred;
}

void distancia(){
  if (end == 1) {
    const int numMeasurements = 15; //NÚMERO DE MEDICIONES PARA EL PROMEDIO
    int rssi = WiFi.RSSI();
    float pred = predict(rssi);

    distanceSum += pred;
    measurementCount++;

    if (measurementCount >= numMeasurements) {
      float averageDistance = distanceSum / numMeasurements;
      Serial.print("RSSI: ");
      Serial.print(rssi);
      Serial.print("   PROMEDIO DISTANCIA: ");
      Serial.println(averageDistance);

      if (pred > 250) {
        stepper.enableOutputs();
        stepper.setSpeed(1000);
        bot.sendMessage(CHAT_ID, "SU PERRO SE HA ESCAPADO!", "");
        delay(3000); 
        int pasos = (90 * stepsPerRevolution) / 360;
        stepper.moveTo(pasos);
        while (stepper.distanceToGo() != 0) {
         stepper.run();
        }
        stepper.setSpeed(0);
        stepper.disableOutputs();
      }

      distanceSum = 0;
      measurementCount = 0;
      delay(1000);
    }
  } 

}
