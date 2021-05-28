#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <Arduino_JSON.h>

// wifi variables
const char* ssid = "HANA SUKARNO";
const char* password = "zirly792";

String serverName = "http://192.168.1.12:3001/api";

// max 65536
//// read sensor every 60s
//#define READ_PERIOD 60
#define READ_PERIOD 1

// max 65536
//// send every 1800s
//#define COMM_PERIOD 1800
#define COMM_PERIOD 10

uint16_t denom = COMM_PERIOD / READ_PERIOD;

const int ledPinR = 13;
const int ledPinB = 12;

// setting PWM properties
const int freq = 5000;
const int ledChannelR = 0;
const int ledChannelB = 1;
const int resolution = 16;
int dutyCycleB = 0;
int dutyCycleR = 0;

uint32_t accumBlue = 0;
uint32_t accumRed = 0;

// sensor settings
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

// timer settings
volatile int interruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint32_t seconds = 0;


void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void postReading(uint16_t r, uint16_t b) {
  HTTPClient http;
      
  // Your Domain name with URL path or IP address with path
  http.begin(serverName + "/input");

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(
    String("{\"id\":1,\"values\": {\"r\":")
    + String(r)
    + String(",\"b\":")
    + String(b)
    + String("}}")
  );
 
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
    
  // Free resources
  http.end();
  
}

String httpGETRequest(String serverName) {
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void getTarget(uint16_t * r, uint16_t * b) {
  String url = serverName + String("/output/1");
  String httpResponse = httpGETRequest(url);
  
  JSONVar myObject = JSON.parse(httpResponse);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  Serial.print("JSON object = ");
  Serial.println(myObject);

  JSONVar values = myObject["values"];
  *r = int(values["r"]);
  *b = int(values["b"]);

}

void sendRequest(uint16_t readR, uint16_t readB, uint16_t * targetR, uint16_t * targetB) {
  postReading(readR,readB);
  getTarget(targetR, targetB);
}

void connectToWifi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);

  ledcSetup(ledChannelR, freq, resolution);
  ledcSetup(ledChannelB, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPinB, ledChannelB);
  ledcAttachPin(ledPinR, ledChannelR);

  connectToWifi();
}

void readSensor(uint16_t * R, uint16_t * B) {
    uint16_t G, C;
    tcs.getRawData(R, &G, B, &C);
    accumBlue += *B;
    accumRed += *R;
}

uint16_t targetR;
uint16_t targetB;

void secondsIncrement() {
    uint16_t R, B;
    
    seconds++;

    if (seconds == 86400) {
      seconds = 0;
      }
 
    Serial.print("Timer: ");
    Serial.print(seconds / 60);
    Serial.print(":");
    Serial.println(seconds % 60);

    if ( (seconds % READ_PERIOD == 0) ) readSensor(&R, &B);
    
    if ( (seconds % COMM_PERIOD == 0) ) {
      uint16_t averageR = accumRed / denom;
      uint16_t averageB = accumBlue / denom;

      sendRequest(averageR, averageB, &targetR, &targetB);
      Serial.print("returned r = "); Serial.print(targetR);
      Serial.print(" b = "); Serial.print(targetB);
      Serial.println(" ");
      
      accumRed = 0;
      accumBlue = 0;

    } 

    int errorB = targetB - B; // positif: LED kurang terang ; negatif: LED terlalu terang
    Serial.print("errorB = "); Serial.print(errorB);

    int errorR = targetR - R; // positif: LED kurang terang ; negatif: LED terlalu terang
    Serial.print(" errorR = "); Serial.println(errorR);
    
    dutyCycleB = errorB + dutyCycleB;
    if (dutyCycleB < 0) dutyCycleB = 0;
    else if (dutyCycleB > 65535) dutyCycleB = 65535;
    Serial.print("dcB = "); Serial.print(dutyCycleB);
    
    dutyCycleR = errorR + dutyCycleR;
    if (dutyCycleR < 0) dutyCycleR = 0;
    else if (dutyCycleR > 65535) dutyCycleR = 65535;
    Serial.print(" dcR = "); Serial.println(dutyCycleR);
    
    ledcWrite(ledChannelB, dutyCycleB);
    ledcWrite(ledChannelR, dutyCycleR);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (interruptCounter > 0) {
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

    secondsIncrement();
  }
}
