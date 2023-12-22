#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

#define DHTPIN 3
#define button 2
#define relayN2 9
#define relayN1 10
#define sensorPower 7
#define btnUp 6
#define btnDown 5
#define soilMoisture A0
#define DHTTYPE DHT11
#define SCREEN_WIDTH 128    // OLED display width,  in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS1307 rtc;
int waterLevel = 99;
int soilAdjustment = 30;

volatile byte buttonReleased = false;  

void setup() {
	Serial.begin(9600);
  Wire.begin();
	pinMode(sensorPower, OUTPUT);
  pinMode(relayN1, OUTPUT);
  pinMode(relayN2, OUTPUT);
	digitalWrite(sensorPower, LOW);
  dht.begin();
  pinMode(button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), turnOnPump, FALLING);
  pinMode(btnUp, INPUT);
  pinMode(btnDown, INPUT);
  
// Tempo 
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
  //  rtc.adjust(DateTime(2023, 10, 16, 6, 38, 0));
  }
  rtc.adjust(DateTime(2023, 10, 16, 7, 0, 20));///

// Start diplay OLED with the address 0x3C to 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

}

void loop() {
  
  if (buttonReleased) {
    buttonReleased = false;
    digitalWrite(relayN1, LOW);
  }
  
  byte btnUpState = digitalRead(btnUp);
  byte btnDownState = digitalRead(btnDown);
  
  DateTime now = rtc.now();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

// Soil Moisture Sensor
  if (now.hour()%8 == 0 && now.minute()*1 == 0 && (now.second()*1 == 0 || now.second()/1 == 1)){
    waterLevel = readSensor();
  }

// Verify if the DHT sensor is working
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Falha ao ler o sensor DHT!"));
    return;
  }
  
  if(btnUpState == HIGH){
     soilAdjustment = soilAdjustment + 5;
  }

  if(btnDownState == HIGH){
    soilAdjustment = soilAdjustment - 5;
  }
    
//  float hic = dht.computeHeatIndex(t, h, false); // Heat Index
	Serial.print("Umidade do Solo: "); Serial.print(waterLevel); Serial.println("%");
  Serial.print("Umidade do Ar: "); Serial.print(h); Serial.println("%");
  Serial.print("Temperatura: "); Serial.print(t); Serial.println("°C");
  Serial.print("Horário: "); Serial.print(now.hour()); Serial.print(":"); Serial.print(now.minute()); Serial.print(":"); Serial.println(now.second());

// Lamp
  if (now.hour() >= 7 && now.hour() <= 19){
    digitalWrite(relayN2, LOW);
  } else {
    digitalWrite(relayN2, HIGH);
  }

// Water Pump
  if ((waterLevel < soilAdjustment) && (now.hour() == 10 || now.hour() == 22) && (now.minute() == 1) && (now.second() >= 1 && now.second() <= 5)){
    digitalWrite(relayN1, LOW);
  } else{
    digitalWrite(relayN1, HIGH);
  }

// OLED Display
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 8);
  oled.print("Temperatura: "); oled.print(t); oled.println("C");
  oled.setCursor(0, 17);
  oled.print("Umidade do Ar: "); oled.print(h); oled.println("%"); 
  oled.setCursor(0, 26);
  oled.print("Umidade do Solo: "); oled.print(waterLevel); oled.println("%");
  oled.setCursor(0, 35);
  oled.print("Umidade Ajus.: "); oled.print(soilAdjustment); oled.println("%");
  oled.setCursor(0, 44);
  oled.print("Hora: "); oled.print(now.hour()); oled.print(":"); oled.print(now.minute()); oled.print(":"); oled.println(now.second());
  oled.display();

  delay(1000);
}

//  Function to measure the soil moisture
int readSensor() {
	digitalWrite(sensorPower, HIGH);
	delay(10);
  int val = map(analogRead(soilMoisture), 315, 1007, 100, 0);
	digitalWrite(sensorPower, LOW);
	return val;
}

// Function to activate the pump manually (interruption button)
void turnOnPump(){
  buttonReleased = true;
}