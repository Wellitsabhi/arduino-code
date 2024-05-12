//Firebase Configuration--------------------------------------
#include<WiFi.h>
#include<Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define WIFI_SSID "My device"
#define WIFI_PASSWORD "12344321"
#define API_KEY "AIzaSyC6PsQgnGNwQTQkOpTKOMO4MuQ7yQzYiVI"
#define DATABASE_URL "https://test-1-bcddf-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
//-------------------------------------------------------------

namespace pin {
const byte tds_sensor = A0;
const byte one_wire_bus = 13; // Dallas Temperature Sensor
}
#define LED_Pin 2

//Temperature Sensor-------------------------------------------
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(pin::one_wire_bus);
DallasTemperature dallasTemperature(&oneWire);
//-------------------------------------------------------------

namespace device {
float aref = 3.3; // Vref, this is for 3.3v compatible controller boards, for arduino use 5.0v.
}

namespace sensor {
float ec = 0;
unsigned int tds = 0;
float waterTemp = 25;
float ecCalibration = 1;
}

float tdsData = 0;
bool ledStatus = false;
float Celsius = 0;
float Fahrenheit = 0;
float ecData = 0;

void setup() {
  Serial.begin(115200); // Dubugging on hardware Serial 0
  dallasTemperature.begin();

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

  delay(2000);
}


void loop() {
  // ecData = readTdsQuick();
  // tdsData = (133.42 * pow(ecData, 3) - 255.86 * ecData * ecData + 857.39 * ecData) * 0.05;
  // dallasTemperature.requestTemperatures();
  // Celsius = dallasTemperature.getTempCByIndex(0);
  // Fahrenheit = (Celsius * 9 / 5) + 32; 

  Serial.println("--------------------------------");
  Serial.print("EC data : ");
  Serial.print(ecData); // mS/cm
  Serial.print(" | ");
  Serial.print("TDS Data : ");
  Serial.print(tdsData); // ppm
  Serial.print(" | ");
  Serial.print("Celsius : ");
  Serial.print(Celsius);
  Serial.print(" | ");
  Serial.print("Fahrenheit : ");
  Serial.println(Fahrenheit);


  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //-------------Store sensor data to RTDB
    ecData = readTdsQuick();
    tdsData = (133.42 * pow(ecData, 3) - 255.86 * ecData * ecData + 857.39 * ecData) * 0.04;
    dallasTemperature.requestTemperatures();
    Celsius = dallasTemperature.getTempCByIndex(0);
    Fahrenheit = (Celsius * 9 / 5) + 32; 

    Serial.println("--------------------------------");
    Serial.print("EC data : ");
    Serial.print(ecData); // mS/cm
    Serial.print(" | ");
    Serial.print("TDS Data : ");
    Serial.print(tdsData); // ppm
    Serial.print(" | ");
    Serial.print("Celsius : ");
    Serial.print(Celsius);
    Serial.print(" | ");
    Serial.print("Fahrenheit : ");
    Serial.println(Fahrenheit);


    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "Sensor/tdsData", tdsData)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
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
    if (Firebase.RTDB.setFloat(&fbdo, "sensors/ec", ecData)){
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

float readTdsQuick() {
  dallasTemperature.requestTemperatures();
  sensor::waterTemp = dallasTemperature.getTempCByIndex(0);
  float rawEc = analogRead(pin::tds_sensor) * device::aref / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
  float temperatureCoefficient = 1.0 + 0.02 * (sensor::waterTemp - 25.0); // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  sensor::ec = (rawEc / temperatureCoefficient) * sensor::ecCalibration; // temperature and calibration compensation
  sensor::tds = (133.42 * pow(sensor::ec, 3) - 255.86 * sensor::ec * sensor::ec + 857.39 * sensor::ec) * 0.03; //convert voltage value to tds value
  return(sensor::ec); 
}