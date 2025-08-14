// EN2853 - Embedded Systems and Applications
// Smart MediBox - Programming Assignment 2
// Akindu Induwara - 210113L

//import required libraries
#include <WiFi.h>
#include <PubSubClient.h>

#include "DHTesp.h"
#include <ESP32Servo.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//define parameters required by oled display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

//define pins for componenets
#define buzzer 5
#define led1 15
#define pb_cancel 34
#define pb_ok 25
#define pb_up 26
#define pb_down 35
#define dhtpin 12
#define servo_pin 27
#define ldr_right 32
#define ldr_left 33

//define parameter used get time from internet
#define NTP_SERVER     "pool.ntp.org"
int UTC_OFFSET = 19800;
#define UTC_OFFSET_DST 0

//declare variables to store alarm info
bool alarm_enabled = true;
int n_alarms = 2;
int alarm_hours[]={0,1};
int alarm_minutes[] = {1,10};
bool alarm_triggered[] = {false,false};

// initialize object for relevent components and services
WiFiClient espClient; 
PubSubClient mqttClient(espClient);
Servo servo; 
DHTesp dhtSensor;
// configure oled display
Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);


//arrays used to tranfer data using mqtt
char tempAr[6];
char humidAr[6];
char luxAr[6];
char LightDir[10];
char ctrl_Factor[6];
char min_Angle[6];

//servo control variables
float minAngle = 30;
float ctrlFactor = 0.75;

//varibles to store ldr input
float ValueR;
float ValueL;

//vaiables to store date,time info
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;
unsigned int timeNow = 0;
unsigned int timeLast = 0;

//varibles to store buzzer tones
int n_notes=8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B  =494;
int C_H = 523;
int notes[] = {C,D,E,F,G,A,B,C_H};

//variables to menu options
int current_mode=0;
int max_modes=4;
String mode_list[]={"1-Set Time","2-Set Alarm 1","3-Set Alarm 2","4-Disable Alarms"};


void setup() {
  Serial.begin(115200);
  //configure component pins
  pinMode(buzzer,OUTPUT);
  pinMode(led1,OUTPUT);
  pinMode(pb_cancel,INPUT);
  pinMode(pb_ok,INPUT);
  pinMode(pb_up,INPUT);
  pinMode(pb_down,INPUT);
  pinMode(ldr_left, INPUT);
  pinMode(ldr_right, INPUT);
  // configure servo
  servo.attach(servo_pin);
  // configure dht sensor
  dhtSensor.setup(dhtpin,DHTesp::DHT22);
  // start oled display
  if (!display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS)){
    Serial.println("SSD1306 Allocation Failed");
    for(;;);
  }
  display.display();
  delay(500);
  SetupWiFi();
  display.clearDisplay();
  print_line("Connected to WiFi",0,0,2);

  configTime(UTC_OFFSET,UTC_OFFSET_DST,NTP_SERVER);
  
  display.clearDisplay();
  print_line("Welcome to MediBox!",0,0,2);
  delay(2000);
  display.clearDisplay();
  setupMqtt();
}

void loop() {
  update_time_with_check_alarm();
  if(digitalRead(pb_ok)==LOW){
    delay(200);
    goto_menu();
  }
  mqttFunc();
}
// function to set mqtt server and call back procedure
void setupMqtt(){
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(recieveCallback);
}
// function to star mqtt communication processes
void mqttFunc(){
  if(!mqttClient.connected()){
    connectToBroker();
  }
  mqttClient.loop();
  updateTemp();
  updateLux();
  Serial.print("luxAr ");Serial.println(luxAr);
  //Serial.println(LightDir);
  mqttClient.publish("MEDIBOX-LUX",luxAr);
  delay(1000);
}
// funtion to connect to wifi network
void SetupWiFi(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println("Wokwi-GUEST");
  WiFi.begin("Wokwi-GUEST","");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    display.clearDisplay();
    print_line("Connecting to WiFi...",0,0,2);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
// function to connect o mqtt broker, subcribe and publish topics
void connectToBroker(){
  while(!mqttClient.connected()){
    Serial.println("Attempting MQTT Connection");
    if(mqttClient.connect("ESP-123456789")){
      Serial.println("Connected to MQTT Broker");
      mqttClient.subscribe("MEDIBOX-ANGLE");
      mqttClient.subscribe("MEDIBOX-CTRLFACTOR");
      mqttClient.subscribe("MEDIBOX-ALARM1");
      mqttClient.subscribe("MEDIBOX-ALARM2");
      mqttClient.subscribe("MEDIBOX-ALARM3");
      String(minAngle).toCharArray(min_Angle, 6);
      String(ctrlFactor).toCharArray(ctrl_Factor, 6);
      mqttClient.publish("MEDIBOX-CTRLFACTOR", ctrl_Factor);
      mqttClient.publish("MEDIBOX-ANGLE", min_Angle);
    }
    else{
      Serial.print("Failure! Code ");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}
// function to hadle imcoming mqtt messages
void recieveCallback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message Recieved [");
  Serial.print(topic);
  Serial.print("] ");
  char payloadCharAr[length];
  for(int i=0;i<length;i++){
    Serial.print((char)payload[i]);
    payloadCharAr[i]=(char)payload[i];
  }
  if(strcmp(topic,"MEDIBOX-ANGLE")==0){
    minAngle = atof(payloadCharAr);
    updateServoPos();
  }
  else if (strcmp(topic,"MEDIBOX-CTRLFACTOR")==0){
    ctrlFactor = atof(payloadCharAr);
    updateServoPos();
  }
  else if (strcmp(topic,"MEDIBOX-ALARM1")==0){
    if(payloadCharAr[0] == '1'){
      alarm_triggered[0] = true;
      alarm_enabled = true;
    }else{
      alarm_triggered[0] = false;
    }
  }
  else if (strcmp(topic,"MEDIBOX-ALARM2")==0){
    if(payloadCharAr[0] == '1'){
      alarm_triggered[1] = true;
      alarm_enabled = true;
    }else{
      alarm_triggered[1] = false;
    }
  }
}

//function to update temperature and humidity, 
// dislay it in oled and send data to node red dashboard
void updateTemp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  String(data.temperature,2).toCharArray(tempAr,6);
  String(data.humidity,2).toCharArray(humidAr,6);
  Serial.print("tempAr ");Serial.println(tempAr);
  Serial.print("humidAr ");Serial.println(humidAr);
  mqttClient.publish("MEDIBOX-TEMP",tempAr);
  mqttClient.publish("MEDIBOX-HUMIDITY",humidAr);
  if(data.temperature > 35){
    display.clearDisplay();
    print_line("TEMP HIGH",0,40,1);
    delay(200);
  }
  else if(data.temperature < 25){
    display.clearDisplay();
    print_line("TEMP LOW",0,40,1);
    delay(200);
  }
  if(data.humidity > 40){
    display.clearDisplay();
    print_line("HUMIDITY HIGH",0,50,1);
    delay(200);
  }
  else if(data.humidity < 20){
    display.clearDisplay();
    print_line("HUMIDITY LOW",0,50,1);
    delay(200);
  }
  
}

// funtion to update light Intensity and send data to node-red dashboard
void updateLux(){
  analogReadResolution(10);
  ValueL = analogRead(ldr_left);
  ValueR = analogRead(ldr_right);
  ValueL = 1 - (ValueL / 1023); //esp32 ADC resolution is 12bits therefore the input is divided by 2^12-1
  ValueR = 1 - (ValueR / 1023); 
  if(ValueL >= ValueR){
    String(ValueL,2).toCharArray(luxAr,6);
    strcat(LightDir,":Left");
  }
  else{
    String(ValueR,2).toCharArray(luxAr,6);
    strcat(LightDir,":Right");
  }
  updateServoPos();
  Serial.print("luxAr ");Serial.println(luxAr);
  mqttClient.publish("MEDIBOX-LUX",luxAr);
}
// funtion to update servo position
void updateServoPos(){
  float D = 0;
  if(ValueL >= ValueR){
    D=1.5;
  }
  else{
    D=0.5;
  }
  int angle = min( int(minAngle*D + (180-minAngle)*max(ValueR,ValueL)*ctrlFactor), 180);
  servo.write(angle);
}
// function to display a line of text in oled display
void print_line(String text, int row, int column, int text_size){
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(row,column);
  display.println(text);
  display.display();
}
// function to display current time in oled display
void print_time_now(void){
  display.clearDisplay();
  print_line(String(days),0,0,2);
  print_line(":",20,0,2);
  print_line(String(hours),30,0,2);
  print_line(":",50,0,2);
  print_line(String(minutes),60,0,2);
  print_line(":",80,0,2);
  print_line(String(seconds),90,0,2);
}
// funtion to update time
void update_time(){
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char timeHour[8];
  strftime(timeHour,8,"%H",&timeinfo);
  hours = atoi(timeHour);
  char timeMinute[8];
  strftime(timeMinute,8,"%M",&timeinfo);
  minutes = atoi(timeMinute);
  char timeSecond[8];
  strftime(timeSecond,8,"%S",&timeinfo);
  seconds = atoi(timeSecond);
  char timeDay[8];
  strftime(timeDay,8,"%d",&timeinfo);
  days = atoi(timeDay);
}
// funvtion that hanlfes alarm rining procedures
void ring_alarm(){
  display.clearDisplay();
  print_line("Medicine Time",0,0,2);
  digitalWrite(led1,HIGH);
  bool break_happened = false;
  while(break_happened == false && digitalRead(pb_cancel)==HIGH){
    for (int i=0; i<n_notes; i++){
      if(digitalRead(pb_cancel)==LOW){
        delay(200);
        break_happened = true;
        break;
      }
      tone(buzzer,notes[i]);
      delay(500);
      noTone(buzzer);
      delay(2);
    }
  }
  digitalWrite(led1,LOW);
  display.clearDisplay();
}

void update_time_with_check_alarm(){
  display.clearDisplay();
  update_time();
  print_time_now();
  if(alarm_enabled == true){
    for (int i=0; i<n_alarms; i++){
      if(alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes){
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}

int wait_for_button_press(){
  while (true){
    if (digitalRead(pb_up) == LOW){
      delay(200);
      return pb_up;
    }
    if (digitalRead(pb_down) == LOW){
      delay(200);
      return pb_down;
    }
    if (digitalRead(pb_ok) == LOW){
      delay(200);
      return pb_ok;
    }
    if (digitalRead(pb_cancel) == LOW){
      delay(200);
      return pb_cancel;
    }
    update_time();
  }
}

void goto_menu(){
  while (digitalRead(pb_cancel)==HIGH){
    display.clearDisplay();
    print_line(mode_list[current_mode],0,0,2);
    int pressed = wait_for_button_press();
    
    if (pressed == pb_up){
      delay(200);
      current_mode +=1;
      current_mode = current_mode % max_modes;
    }
    else if (pressed == pb_down){
      delay(200);
      current_mode -=1;
      if(current_mode<0){
        current_mode=max_modes-1;
      }
    }
    else if (pressed == pb_ok){
      delay(200);
      run_mode(current_mode);
      Serial.println(current_mode);
    }
    else if (pressed == pb_cancel){
      delay(200);
      break;
    }
  }
}

void run_mode(int mode){
  if(mode == 0){
    set_time();
  }
  else if(mode == 1 || mode == 2){
    set_alarm(mode-1);
  }
  else if (mode == 3){
    alarm_enabled = false;
  }
}

void set_alarm(int alarm){
  int temp_hour = alarm_hours[alarm];
  while (true){
    display.clearDisplay();
    print_line("Enter alarm hour: " + String(temp_hour),0,0,2);
    int pressed = wait_for_button_press();
    if (pressed == pb_up){
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == pb_down){
      delay(200);
      temp_hour -= 1;
      if (temp_hour<0){
        temp_hour = 23;
      }
    }
    else if (pressed == pb_ok){
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }
    else if (pressed == pb_cancel){
      delay(200);
      break;
    }
  }
  int temp_minute = alarm_minutes[alarm];
  while (true){
    display.clearDisplay();
    print_line("Enter alarm_minutes: " + String(temp_minute),0,0,2);
    int pressed = wait_for_button_press();
    if (pressed == pb_up){
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == pb_down){
      delay(200);
      temp_minute -= 1;
      if (temp_minute<0){
        temp_minute = 59;
      }
    }
    else if (pressed == pb_ok){
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    }
    else if (pressed == pb_cancel){
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Alarm is set",0,0,2);
  delay(1000);
}

void set_time(){
  int temp_hour = hours;
  while (true){
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour),0,0,2);
    int pressed = wait_for_button_press();
    if (pressed == pb_up){
      delay(200);
      temp_hour += 1;
      UTC_OFFSET +=3600;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == pb_down){
      delay(200);
      temp_hour -= 1;
      UTC_OFFSET -=3600;
      if (temp_hour<0){
        temp_hour = 23;
      }
    }
    else if (pressed == pb_ok){
      delay(200);
      hours = temp_hour;
      break;
    }
    else if (pressed == pb_cancel){
      delay(200);
      break;
    }
  }

  int temp_minute = minutes;
  while (true){
    display.clearDisplay();
    print_line("Enter minutes: " + String(temp_minute),0,0,2);
    int pressed = wait_for_button_press();
    if (pressed == pb_up){
      delay(200);
      temp_minute += 1;
      UTC_OFFSET +=60;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == pb_down){
      delay(200);
      temp_minute -= 1;
      UTC_OFFSET -=60;
      if (temp_minute<0){
        temp_minute = 59;
      }
    }
    else if (pressed == pb_ok){
      delay(200);
      minutes = temp_minute;
      break;
    }
    else if (pressed == pb_cancel){
      delay(200);
      break;
    }
  }
  configTime(UTC_OFFSET,UTC_OFFSET_DST,NTP_SERVER);
  display.clearDisplay();
  print_line("Time is set",0,0,2);
  delay(1000);
}

