/**************** Programa: Casa Monitorada - Automatic irrigator * 
Autor: Felipe Simão * 
Descrição: Programa de monitoramento e controle residencial através de aplicativo * 
em comunicação MQTT com o ESP8266 
******************************************************************************/


#include <ESP8266WiFi.h> 
#include <PubSubClient.h> 
#define MSG_BUFFER_SIZE (50) 
char msg[MSG_BUFFER_SIZE];                                                                        
#include <Adafruit_Sensor.h>
#include <DHT.h> 
#include <DHT_U.h>
#define DHTPIN 5 
#define DHTTYPE DHT22 
DHT_Unified dht(DHTPIN, DHTTYPE);                                                                 
uint32_t delayMS;                                                                                 
const char* ssid = "wi-fi";                                                          
const char* password = "password";                                                              
const char* mqtt_server = "broker.mqtt-dashboard.com";                                            
WiFiClient espClient;                                                                             
PubSubClient client(espClient);                                                                   
unsigned long lastMsg = 0;                                                                        
int value = 0;                                                                                    

char bomba;                                                                                        
float umid;                                                                                

void setup() {                                                                                    
  
  dht.begin();                                                                                    
  Serial.println(F("Sensor DHT22"));                                                              
  sensor_t sensor;                                                                                
  
  
  dht.temperature().getSensor(&sensor);                                                           
  Serial.println(F("------------------------------------"));                                      
  Serial.println(F("Temperatura"));                                                               
  Serial.print  (F("Sensor: ")); Serial.println(sensor.name);                                     
  Serial.print  (F("Valor máximo: ")); Serial.print(sensor.max_value); Serial.println(F("°C")); 
  Serial.print  (F("Valor mínimo: ")); Serial.print(sensor.min_value); Serial.println(F("°C")); 
  Serial.print  (F("Resolução: ")); Serial.print(sensor.resolution); Serial.println(F("°C"));    
  Serial.println(F("------------------------------------"));                                      
 
  
  dht.humidity().getSensor(&sensor);                                                              
   Serial.println(F("------------------------------------"));                                     
  Serial.println(F("Umidade"));                                                                   
  Serial.print  (F("Sensor: ")); Serial.println(sensor.name);                                     
  Serial.print  (F("Valor máximo: ")); Serial.print(sensor.max_value); Serial.println(F("%"));  
  Serial.print  (F("Valor mínimo: ")); Serial.print(sensor.min_value); Serial.println(F("%"));  
  Serial.print  (F("Resolução: ")); Serial.print(sensor.resolution); Serial.println(F("%"));     
  Serial.println(F("------------------------------------"));                                      
  
  delayMS = sensor.min_delay / 1000;        
  
  bomba = 4;                                                                                       
                                                                                 

  pinMode(bomba, OUTPUT);                                                                          
                                                                     
  
  Serial.begin(115200);                                                                           
  
  setup_wifi();                                                                                   
  client.setServer(mqtt_server, 1883);                                                            
  client.setCallback(callback);                                                                   
}

void setup_wifi() {                                                                               

  delay(10);                                                                                      
  Serial.println("");                                                                             
  Serial.print("Conectando com ");                                                                
  Serial.println(ssid);                                                                           

  WiFi.begin(ssid, password);                                                                     

  while (WiFi.status() != WL_CONNECTED) {                                                         
    delay(500);                                                                                   
    Serial.print(".");                                    
  }                  

  Serial.println("");                                                                             
  Serial.println("WiFi conectado");                                                               
  Serial.println("IP: ");                                                                         
  Serial.println(WiFi.localIP());                                                                 
}

void callback(char* topic, byte* payload, unsigned int length) {                                  
  Serial.print("Mensagem recebida [");                                                            
  Serial.print(topic);                                                                            
  Serial.print("] ");                                                                             
  for (int i = 0; i < length; i++) {                                                              
    Serial.print((char)payload[i]);                                                               
  }
  Serial.println("");                                                                             
  if ((char)payload[0] == 'B') {                                                                  
    digitalWrite(bomba, HIGH);                                                                     
                                   
                                                                
  }
  Serial.println("");                                                                             
  if ((char)payload[0] == 'b') {                                                                  
    digitalWrite(bomba, LOW);                                                                      
                                                            
  }
 
}

void reconnect() {                                                                                
  while (!client.connected()) {                                                                   
    Serial.print("Aguardando conexão MQTT...");                                                   
    String clientId = "ESP8266Client";                                       
    clientId += String(random(0xffff), HEX);                                                      
    if (client.connect(clientId.c_str())) {                                                       
      Serial.println("conectado");                                                                
      client.subscribe("Irrigador/publisher");                                                         
    } else {                                                                                      
      Serial.print("falhou, rc=");                                                                
      Serial.print(client.state());                                                               
      Serial.println("tente novamente em 5s");                                                    
      delay(5000);                                                                                
    }
  }
}

void loop() {                                                                                     
  delay(delayMS);                                                                                 
  sensors_event_t event;                                                                          
  dht.temperature().getEvent(&event);                                                             
  if (isnan(event.temperature)) {                                                                 
    Serial.println(F("Erro na leitura da temperatura!"));                                         
  }
  else {                                                                                          
    Serial.print(F("Temperatura: "));                                                             
    Serial.print(event.temperature);                                                              
    Serial.println(F("°C"));                                                                      
    sprintf(msg,"%f",event.temperature);                                                          
    client.publish("Irrigador/temperatura", msg);                                                      
  }
  dht.humidity().getEvent(&event);                                                                
  if (isnan(event.relative_humidity)) {                                                           
    Serial.println(F("Erro na leitura da umidade!"));                                             
  }
  else {                                                                                          
    Serial.print(F("Umidade: "));                                                                 
    Serial.print(event.relative_humidity);                                                        
    Serial.println(F("%"));                                                                       
    sprintf(msg,"%f",event.relative_humidity);                                                    
    client.publish("Irrigador/umidade", msg);                                                          
  }
  umid=event.relative_humidity;
  if(umid<70){
    digitalWrite(bomba, HIGH);
    
    
 }
   if(umid>=80){
    digitalWrite(bomba, LOW);
    
    
  }
  
  if (!client.connected()) {                                                                      
    reconnect();                                                                                  
  }
  client.loop();                                                                                  
}
