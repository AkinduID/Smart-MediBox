#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>

#include <functions.h>
#include <globals.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define buzzer 5
#define led1 15
#define pb_cancel 34
#define pb_ok 32
#define pb_up 33
#define pb_down 35
#define dhtpin 12

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     0
#define UTC_OFFSET_DST 0

bool alarm_enabled = true;
int n_alarms = 2;
int alarm_hours[]={0,1};
int alarm_minutes[] = {1,10};
bool alarm_triggered[] = {false,false};


Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);
DHTesp dhtSensor;
// global variables
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

unsigned int timeNow = 0;
unsigned int timeLast = 0;

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

int current_mode=0;
int max_modes=4;
String mode_list[]={"1-Set Time","2-Set Alarm 1","3-Set Alarm 2","4-Disable Alarms"};


void setup() {
  pinMode(buzzer,OUTPUT);
  pinMode(led1,OUTPUT);
  pinMode(pb_cancel,INPUT);
  pinMode(pb_ok,INPUT);
  pinMode(pb_up,INPUT);
  pinMode(pb_down,INPUT);
  dhtSensor.setup(dhtpin,DHTesp::DHT22);
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Serial.println("Hello, ESP32!");
  if (!display.begin(SSD1306_SWITCHCAPVCC,SCREEN_ADDRESS)){
    Serial.println("SSD1306 Allocation Failed");
    for(;;);
  }
  display.display();
  delay(500);

  WiFi.begin("Wokwi-GUEST", "",6);
  while(WiFi.status()!=WL_CONNECTED){
    delay(200);
    display.clearDisplay();
    print_line("Connecting to WiFi...",0,0,2);
  }

  display.clearDisplay();
  print_line("Connected to WiFi",0,0,2);

  configTime(UTC_OFFSET,UTC_OFFSET_DST,NTP_SERVER);
  
  display.clearDisplay();
  print_line("Welcome to MediBox!",0,0,2);
  delay(2000);
  display.clearDisplay();
}

void loop() {
  update_time_with_check_alarm();
  if(digitalRead(pb_ok)==LOW){
    delay(200);
    goto_menu();
  }
  check_temp();
}

void print_line(String text, int row, int column, int text_size){
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(row,column);
  display.println(text);
  display.display();
}

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

void update_time(){
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char timeHour[3];
  strftime(timeHour,3,"%H",&timeinfo);
  hours = atoi(timeHour);
  char timeMinute[3];
  strftime(timeMinute,3,"%M",&timeinfo);
  minutes = atoi(timeMinute);
  char timeSecond[3];
  strftime(timeSecond,3,"%S",&timeinfo);
  seconds = atoi(timeSecond);
  char timeDay[3];
  strftime(timeDay,3,"%d",&timeinfo);
  days = atoi(timeDay);
}

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
      delay(500);
    }
  }
  digitalWrite(led1,LOW);
  display.clearDisplay();
}

void update_time_with_check_alarm(){
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
      minutes = temp_minute;
      break;
    }
    else if (pressed == pb_cancel){
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Time is set",0,0,2);
  delay(1000);
}

void check_temp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
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