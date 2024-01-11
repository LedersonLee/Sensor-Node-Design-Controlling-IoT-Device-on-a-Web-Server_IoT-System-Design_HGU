#include "DHTesp.h"
#include <Adafruit_MLX90614.h> //적외선 온도센서 헤더파일
//OLED display를 위한 헤더파일
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "EspMQTTClient.h"

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13 
#define D8 15

#define DHT_PIN D3
#define LED_PIN D0
#define SCL_PIN D1
#define SDA_PIN D2
#define CDS_PIN A0
#define RELAY_PIN D4
#define TRIG D8
#define ECHO D7
#define DHTTYPE DHTesp::DHT22
//display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin
#define SCREEN_ADDRESS 0x3C //See datasheet for Address; 0x3C for 128x32

#define RELAY_ON LOW
#define RELAY_OFF HIGH
#define LED_ON HIGH 
#define LED_OFF LOW 
#define ON 1
#define OFF 0
#define DELAY 1000

#define mqtt_broker "203.252.106.154"
#define mqtt_username "iot"
#define mqtt_password "csee1414"
#define mqtt_clientname "Group6"

void Measure_dist();
void displaytext(String str);
void displayTemp(String str);
void Measure_temp();
void onTestMessageReceived(const String& payload);
void onConnectionEstablished();

DHTesp dht;
float humidity, temperature;
unsigned long prev_mils[2]={0, 0}; //0:for dht, 1:for USBLED
unsigned long cur_mils[2]={0, 0};  //0:for dht, 1:for USBLED
const long delay_DHT=3000;
const long delay_USBLED=10000;
int readDht;
int light_val;
int relayState=RELAY_OFF;
int ledState= LED_OFF;
int readState = ON;
boolean beforeBright=false;
boolean bright= false;
boolean dark=false;
float duration, distance, objectTemp;
unsigned long cur_mils_dist=0;
unsigned long cur_mils_display=0;
bool flag = false;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char *wifi_ssid = "hotplace";
const char *wifi_password = "12088021!!"; 

//Publish를위한Topic
const char *pub_dht= "iot/6/dht22"; 
const char *pub_dht_t= "iot/6/dht22_t"; 
const char *pub_dht_h= "iot/6/dht22_h"; 
const char *pub_cds= "iot/6/cds"; 

//Subscribe Topic
const char *sub_topic= "iot/6";
 
// MQTT
EspMQTTClient client(
  wifi_ssid,
  wifi_password,
  mqtt_broker,
  mqtt_username,
  mqtt_password,
  mqtt_clientname,
  1883              // The MQTT port, default to 1883. this line can be omitted
);


void setup() {
  dht.setup(DHT_PIN, DHTTYPE);
  readDht=OFF;
  
  Serial.begin(115200);
  mlx.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  
  
  pinMode(TRIG,OUTPUT);
  pinMode(ECHO, INPUT);
 
  pinMode(LED_PIN, OUTPUT);
  pinMode(CDS_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

void loop() {
  client.loop();
  DhtControl();
 
  if(readState == ON)
    relayState=RelayState();
  
  if(relayState == RELAY_ON)
    RelayControl();  
    
  Measure_dist();
  Measure_temp();
}

void DhtControl(void){
  cur_mils[0]=millis();

  if(cur_mils[0] - prev_mils[0] >= delay_DHT){
    readDht=ON;
    prev_mils[0]= cur_mils[0];
  }
  if(readDht){
    humidity = dht.getHumidity();
    temperature = dht.getTemperature();
    
    Serial.println("Ambient Temp: "+String(temperature)+" Humid: "+String(humidity));
    light_val=analogRead(CDS_PIN);
    Serial.println("Light: "+String(light_val));
  }
  
  readDht=OFF;
}


int RelayState(void){
  light_val=analogRead(CDS_PIN);
  
  if(light_val >=700){
    beforeBright=dark;
    
    if((beforeBright == dark) && (dark != false)){
      bright=true;
    }
  }
  else if(light_val <700){
    dark=true;
   
    if(bright==true && dark==true){
      dark=false;
      bright=false;
      beforeBright=false;
      prev_mils[1]=millis();
      return RELAY_ON;
    }
  }

  return RELAY_OFF;
}

void RelayControl(){
  cur_mils[1]=millis();
  if(cur_mils[1]-prev_mils[1] > delay_USBLED){
    relayState=RELAY_OFF; //USBLED OFF
    readState= ON;
  }
  else{
    readState=OFF; //10초가 지날 때까지 더이상 relayState를 읽어들이지 않음
  }
    
  digitalWrite(RELAY_PIN, relayState);
}

void Measure_dist(){
  if(millis()>=cur_mils_dist+DELAY){
    cur_mils_dist+= DELAY;
    //trig
    digitalWrite(TRIG, LOW); //초기화
    delay(2);
    digitalWrite(TRIG, HIGH); //초음파 발산
    delay(10);
    digitalWrite(TRIG, LOW);
  
    //echo
    duration=pulseIn(ECHO, HIGH); //초음파가 돌아오는 시간 측정
    distance= ((float)(340*duration)/10000)/2;
    Serial.println("distance: "+String(distance));
  }
}

void Measure_temp(){
  String str;
  if(distance<=20.0){
    if(distance<=5.0){
      objectTemp=mlx.readObjectTempC();
      objectTemp+=4;
      Serial.println("Temp: "+String(objectTemp));
      
      if(objectTemp>=35.9 && objectTemp<=37.5){
        str="Normal temperature\n: "+String(objectTemp);
        displayTemp(str);
      }
      else{
        str="Abnormal temperature.Retry to measure.\n: "+String(objectTemp);
        displayTemp(str);
      }
    }
    else if(distance>5.0){
      str="Want to measure Temp\nPls come within 5cm";
      displaytext(str);
    }
  }
  else{
    display.clearDisplay();
    display.display();
  }   
}

void displaytext(String str) {
  display.clearDisplay();
  display.setTextSize(0.8);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print(str);
  display.display();  
}

void displayTemp(String str) {
  if(flag ==false){
    cur_mils_display=millis();
    flag= true;
  }

  if(millis()>=cur_mils_display+DELAY){
    flag=false;
    display.clearDisplay();
    display.setTextSize(0.8);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print(str);
    display.display();  
  }
}

void onTestMessageReceived(const String& payload){
  Serial.println("onTestMessageReceived");

  ledState = digitalRead(LED_PIN);
  relayState = digitalRead(RELAY_PIN);
    
  if(payload == "led"){
    digitalWrite(LED_PIN, !ledState);
    Serial.println("Switching LED");
  }
  else if(payload =="ledon"){
    digitalWrite(LED_PIN, LED_ON);
  }
  else if(payload =="ledoff"){
    digitalWrite(LED_PIN, LED_OFF);
  }
  else if(payload =="usbled"){
    digitalWrite(RELAY_PIN, !relayState);
  }
  else if(payload =="usbledon"){
    digitalWrite(RELAY_PIN, RELAY_ON);
  }
  else if(payload =="usbledoff"){
    digitalWrite(RELAY_PIN, RELAY_OFF);      
  }
  else if(payload == "usbled") {
    digitalWrite(RELAY_PIN, !relayState);
  }
}

void onConnectionEstablished() {
  Serial.println("... onConnectEstablished()...");

  client.subscribe(sub_topic, onTestMessageReceived); 
  client.subscribe(pub_dht, [](const String & payload) {
    if(payload == "dht22")
      client.publish(pub_dht, "H: "+String(humidity)+" T: "+String(temperature));         
  });
  client.subscribe(pub_dht_t, [](const String & payload) {
    if(payload == "dht22_t")
      client.publish(pub_dht_t, String(temperature)); 
  });  
  client.subscribe(pub_dht_h, [](const String & payload) {
    if(payload == "dht22_h")
      client.publish(pub_dht_h, String(humidity)); 
  });
  client.subscribe(pub_cds, [](const String & payload) {
    if(payload == "cds")
      client.publish(pub_cds, String(humidity)); 
  });
}
