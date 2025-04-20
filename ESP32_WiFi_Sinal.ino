#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <page.h>

//Insert network credentials
const char* ssid = "Casa-SP";
const char* password = "*Estamos em Casa*";

//Create AsyncWebServer object on port 82
AsyncWebServer server(82);

//Create an Event Source on /events
AsyncEventSource events("/events");

//Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

float sinal;

void getSensorReadings(){
  sinal = (WiFi.RSSI())*-1;
}

//Initialize WiFi
void initWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

String processor(const String& var){
  getSensorReadings();
  if(var == "SINAL"){
    return String(sinal);
  }
  return String();
}



void setup() {
  Serial.begin(115200);
  initWiFi();

  //Handle Web Service
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html, processor);
  });

  //Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    //send event with message "hello!'
    //and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
  
}

void loop() {
  if((millis() - lastTime) > timerDelay){
    getSensorReadings();
    Serial.printf("Sinal = %.2f \n", sinal);

    //Send Events to the Web
    events.send("ping",NULL,millis());
    events.send(String(sinal).c_str(),"sinal",millis());

    lastTime = millis();
  }

}
