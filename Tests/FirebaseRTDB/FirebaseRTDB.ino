#include<WiFi.h>
#include<Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "My device"
#define WIFI_PASSWORD "12344321"
#define API_KEY "AIzaSyC6PsQgnGNwQTQkOpTKOMO4MuQ7yQaazYiVI"
#define DATABASE_URL "https://test-1-bcddf-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define LED_Pin 2
#define TDS_Pin A0
#define TEMP_Pin 13

//------Temperature Sensor---------------
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(TEMP_Pin);
DallasTemperature sensors(&oneWire);
float Celsius = 0;//0
float Fahrenheit =0;
float ec = 0;
//------Close temperature sensor--------



FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
int tdsData = 0;//0
float tempData = 0;
bool ledStatus = false;

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  pinMode(LED_Pin, OUTPUT);
}

void loop(){
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    //-------------Store sensor data to RTDB
    tdsData = analogRead(TDS_Pin);
    sensors.requestTemperatures();
    Celsius = (float)sensors.getTempCByIndex(0);
    Fahrenheit = (float)sensors.toFahrenheit(Celsius);


    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "Sensor/tdsData", tdsData)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.println(tdsData);
      Serial.println("-------------------------------");
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/Celsius", Celsius)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.print(Celsius);
      Serial.print(" C  ");
      Serial.print(Fahrenheit);
      Serial.println(" F");
      Serial.println("-----------------------");
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/Fahrenheit", Fahrenheit)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    //---------------------------------------------------------------

    if (Firebase.RTDB.setFloat(&fbdo, "sensors/fahrenheit", Fahrenheit)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    //----------------------------------------------------------------
    if (Firebase.RTDB.setFloat(&fbdo, "sensors/celsius", Celsius)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    //---------------------------------------------------------------
    if (Firebase.RTDB.setFloat(&fbdo, "sensors/tds", tdsData)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    //----------------------------------------------------------------
    if (Firebase.RTDB.setFloat(&fbdo, "sensors/ec", ec)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    //-----------------CONTROL LED--------------
    if (Firebase.RTDB.getBool(&fbdo, "Led/Control")){
      if (fbdo.dataType() == "boolean") {
      ledStatus = fbdo.boolData();
      Serial.println("Successful READ from " + fbdo.dataPath() + ": " + ledStatus + " (" + fbdo.dataType () + ") ");
      digitalWrite (LED_Pin, ledStatus);
      }
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}
