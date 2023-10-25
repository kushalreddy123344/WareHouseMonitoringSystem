#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <HX711.h>
//35400
//186gms
//calibiration-> 190

const char *ssid = "Galaxy";
const char *password = "807448891";

WebServer server(80);
DHT dht(26, DHT11);

int smokeA0=A0;
int data=0;
const int ldr_pin=32;
const int led_pin=13;
const int led_pin1=12;
const int led_pin2=14;
const int buz_pin=21;
const int Dout=4;
const int Sck=2;
HX711 scale;

void handleRoot() {
  char msg[2000];

  snprintf(msg, 2000,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='4'/>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>\
    <title>Ware House Monitoring</title>\
    <style>\
    html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}\
    h2 { font-size: 3.0rem; }\
    p { font-size: 3.0rem; }\
    .units { font-size: 1.2rem; }\
    .dht-labels{ font-size: 1.5rem; vertical-align:middle; padding-bottom: 15px;}\
    </style>\
  </head>\
  <body>\
      <h2>Ware House Monitoring</h2>\
      <p>\
        <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
        <span class='dht-labels'>Temperature</span>\
        <span>%.2f</span>\
        <sup class='units'>&deg;C</sup>\
      </p>\
      <p>\
        <i class='fas fa-tint' style='color:#00add6;'></i>\
        <span class='dht-labels'>Humidity</span>\
        <span>%.2f</span>\
        <sup class='units'>&percnt;</sup>\
      </p>\
      <p>\
        <i class='fas fa-fire' style='color:#BADA55;'></i>\
        <span class='dht-labels'>Gas Level</span>\
        <span>%d</span>\
      </p>\
      <p>\
        <i class='fas fa-lightbulb' style='color:#66CDAA;'></i>\
        <span class='dht-labels'>Lights</span>\
        <span>%s</span>\
      </p>\
      <p>\
        <i class='fas fa-industry' style='color:#66CDAA;'></i>\
        <span class='dht-labels'>Quantity</span>\
        <span>%d</span>\
      </p>\
  </body>\
</html>",
           readDHTTemperature(), readDHTHumidity(),sendSensor(),led(),load()
          );
  server.send(200, "text/html", msg);
}



void setup(void) {

  pinMode(ldr_pin,INPUT);
  pinMode(led_pin,OUTPUT);
  pinMode(led_pin1,OUTPUT);
  pinMode(led_pin2,OUTPUT);
  pinMode(buz_pin,OUTPUT);
  pinMode(smokeA0,INPUT);
  Serial.begin(115200);
  dht.begin();
  scale.begin(Dout,Sck);
  scale.set_scale(198);
  scale.tare();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  load();
  delay(500);//allow the cpu to switch to other tasks
}

char* led(){
  if(digitalRead(ldr_pin)==1){
    digitalWrite(led_pin,HIGH);
    digitalWrite(led_pin1,HIGH);
    digitalWrite(led_pin2,HIGH);
    Serial.println(digitalRead(ldr_pin));
    return "ON";
  }
  else{
    digitalWrite(led_pin,LOW);
    digitalWrite(led_pin1,LOW);
    digitalWrite(led_pin2,LOW);
    Serial.println(digitalRead(ldr_pin));
    return "OFF";
  }
}

int sendSensor(){
  int data=analogRead(smokeA0);
  Serial.print("Pin A0: ");
  Serial.println(data);
  if(data>600)
    tone(buz_pin,1000);
  else
    noTone(buz_pin);
  return data;
}

float readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    Serial.println(t);
    return t;
  }
}

float readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    Serial.println(h);
    return h;
  }
}

int load(){
  float reading;
  int q;
  if(scale.is_ready()){
    Serial.print("HX711 reading: ");
    reading=round(scale.get_units());
    q=round(reading/40);
    Serial.println(reading);
  }
  else{
    Serial.println("HX711 not found");
  }
  return q;
  }
