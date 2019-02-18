#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <DHTesp.h>

#define soil_humidity 32


/** Initialize DHT sensor */
DHTesp dht;

/** WiFi credentials */
const char* ssid = <SSID_DA_REDE>;
const char* password =  <SENHA_DA_REDE>;

/** API credentials */
const char* userToken = "";

/** Pin numbers */
byte dhtPin = 33;
byte higroPin = 32;


// Setup code
void setup() { 
  Serial.begin(9600);
  Serial.println("Starting...");
  pinMode(soil_humidity, INPUT);
  delay(4000);
  
  // Connect to WiFi network
  WiFi.begin(ssid, password); 
 
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }   
  Serial.println("Connected to the WiFi network");

  HTTPClient http;   

  // Http Request to Login route
  http.begin("https://mycrops.herokuapp.com/api/users/login");
  http.addHeader("Content-Type", "application/json");

  // JSON
  char JSONMessage[] = "{\"email\": \"john@gmail.com\", \"password\": \"123456\"}";

  
  //Send the actual POST request
  byte httpResponseCode = http.POST(JSONMessage);
 
  if(httpResponseCode>0){

  //Get the response to the request
  String response = http.getString();             

  Serial.println("Resposta retornada: " + response);
 
  // JSON Parsing
  StaticJsonBuffer<500> JSONBuffer;                       //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(response); //Parse message
 
  if (!parsed.success()) {   //Check for errors in parsing
 
    Serial.println("Parsing failed");
    delay(5000);
    return;
 
  }
  JSONBuffer.clear();
  //Get Token value
  userToken = parsed["token"];
 
  Serial.print("Token: ");
  Serial.println(userToken);
 
  Serial.println();
  delay(5000);
 
  }else{
    
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
 
  }
 
  http.end();  //Free resources
  
  // Initialize temperature sensor
  dht.setup(dhtPin, DHTesp::DHT11);
}

void loop( )
{  
  // Reading temperature and humidity takes about 2 seconds (slow sensor)
  // Collecting air temperature and humidity
  TempAndHumidity lastValues = dht.getTempAndHumidity();
  
  byte airTemperature = lastValues.temperature;
  byte airHumidity = lastValues.humidity;
  delay(2000);
  int soilHumidity = analogRead(soil_humidity);
  String soilCondition = "";

  if (soilHumidity > 3000) {
    soilCondition = "seco";
  }

  if (soilHumidity < 1700) {
    soilCondition = "encharcado";
  }

  if (soilHumidity < 3000 && soilHumidity > 1700) {
    soilCondition = "úmido";
  }

  Serial.println(airTemperature);
  Serial.println(airHumidity);
  Serial.println(soilHumidity);
  Serial.println("Condicao do solo: " + soilCondition);

  StaticJsonBuffer<600> jsonBuffer;

  // Create the root of the object tree.
  JsonObject& json = jsonBuffer.createObject();

  // Add values in the object
  json["name"] = "Estação";
  json["airTemperature"] = airTemperature;
  json["airHumidity"] = airHumidity;

  // Add a nested array.
  JsonArray& crops = json.createNestedArray("crops");

  StaticJsonBuffer<200> jsonBuffer2;

  JsonObject& crop = jsonBuffer2.createObject();
  crop["id"] = "0001";
  crop["name"] = "Horta 01";
  crop["soilHumidity"] = soilHumidity;
  crop["soilCondition"] = soilCondition;
  
  crops.add(crop);

  // Add a nested array.
  JsonArray& readings = json.createNestedArray("readings");
  
  StaticJsonBuffer<200> jsonBuffer3;

  JsonObject& reading = jsonBuffer3.createObject();
  reading["airTemp"] = airTemperature;
  reading["airHum"] = airHumidity;
  reading["soilHum"] = soilHumidity;
  
  readings.add(reading);

  Serial.println();  

  HTTPClient http;   

  // Http Request to hub create/update route
  http.begin("https://mycrops.herokuapp.com/api/hubs");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", userToken);

  Serial.println("Posting data");
  //Send the actual POST request

  // Convert JsonObject to char*
  char jsonChar[100];
  json.printTo((char*)jsonChar, json.measureLength() + 1);

  Serial.println(jsonChar);
  
  byte httpResponseCode = http.POST(jsonChar);
 
  Serial.println(httpResponseCode);
 
  http.end();  //Free resources
  
  delay(10000);
  
}
