// Original source code: https://wiki.keyestudio.com/KS0429_keyestudio_TDS_Meter_V1.0#Test_Code
// Project details: https://RandomNerdTutorials.com/esp32-tds-water-quality-sensor/
#include<WiFi.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DFRobot_ESP_PH_WITH_ADC.h>    
#include <DFRobot_ESP_EC.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

//TDS Setup-----------------------------------------------
#define TdsSensorPin A0
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point

int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation
//--------------------------------------------------------

#define ONE_WIRE_BUS 13                // this is the gpio pin 13 on esp32.
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DFRobot_ESP_EC ec;
DFRobot_ESP_PH_WITH_ADC ph;
// Adafruit_ADS1115 ads;
float phvoltage, phValue, phtemperature = 25;
float voltage, ecValue;
float printEC, PrintTDS;
// Your WiFi credentials.
// Set password to "" for open networks.
#define WIFI_SSID "My device"
#define WIFI_PASSWORD "12344321"

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void setup(){
  Serial.begin(115200);
  pinMode(TdsSensorPin,INPUT);
}

void loop(){
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    temperature = readTemperature(); 
    printEC = readEC();

    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      
      // Serial.print("voltage:");
      // Serial.print(averageVoltage,2);
      // Serial.print("V   ");
      Serial.print("TDS Value : ");
      Serial.print(tdsValue,0);
      Serial.println("ppm");
      Serial.println("--------------");
      Serial.print("Ec Value : ");
      Serial.println(printEC, 2);
      Serial.println("--------------");
      Serial.println("Temperature : ");
      Serial.println(temperature, 2);
      Serial.println("--------------");
      
    }
  }
}

float readTemperature()
{
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
  
}


float readEC()
{
    voltage = analogRead(A0); // A0 is the gpio 36 
    //Serial.print("voltage:");
    //Serial.println(voltage, 4);

    // temperature = readTemperature();  // read your temperature sensor to execute temperature compensation
    // Serial.print("temperature:");
    // Serial.print(temperature, 1); 
    // Serial.println("^C");

    ecValue = ec.readEC(voltage, temperature); // convert voltage to EC with temperature compensation
    Serial.print("EC:");
    Serial.print(ecValue, 2);
    Serial.println(" ms/cm");
    // Serial.println(ecValue); 
  ec.calibration(voltage, temperature); // calibration process by Serail CMD
  return (ecValue);
}

