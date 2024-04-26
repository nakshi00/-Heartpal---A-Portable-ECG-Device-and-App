#include <SoftwareSerial.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "MAX30100.h"

#define REPORTING_PERIOD_MS 1000

SoftwareSerial BTserial(0, 1); // RX | TX

const long baudRate = 9600; 
char c = ' ';
boolean NL = true;

MAX30100 sensor;
PulseOximeter pox;

uint32_t tsLastReport = 0;
bool fingerOnSensor = false;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
  Serial.println("Beat!");
  fingerOnSensor = true;
}

void setup()
{
  Serial.begin(9600);
  
  BTserial.begin(baudRate);  
  Serial.print("BTserial started at "); Serial.println(baudRate);
  Serial.println(" ");
  Serial.print("Initializing pulse oximeter..");

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply, or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  sensor.begin(); // Get raw values
  sensor.setMode(MAX30100_MODE_SPO2_HR);
  pox.begin();
  pinMode(A0, INPUT); // Set A0 as input for AD8232 ECG sensor
}

void loop()
{
  /*-------------MAX30100---------------------*/
  // Make sure to call update as fast as possible
  sensor.update();
  pox.update();

  // Asynchronously dump heart rate and oxidation levels to the serial
  // For both, a value of 0 means "invalid"
  pox.setOnBeatDetectedCallback(onBeatDetected);
  
  if (fingerOnSensor && millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    Serial.print("bpm / SpO2:");
    Serial.print(pox.getSpO2());
    Serial.println("%");

    tsLastReport = millis();
  }

  /*-------------AD8232---------------------*/
  float ecgValue = analogRead(A0);
  
  if (!fingerOnSensor) {
    float ecgValue = analogRead(A0);
    Serial.println(ecgValue);
  }

  // Read from the Bluetooth module and send to the Arduino Serial Monitor
  if (BTserial.available())
  {
    c = BTserial.read();
    Serial.write(c);
  }

  // Read from the Serial Monitor and send to the Bluetooth module
  if (Serial.available())
  {
    c = Serial.read();
    BTserial.write(c);   

    // Echo the user input to the main window. The ">" character indicates the user entered text.
    if (NL) { Serial.print(">");  NL = false; }
    Serial.write(c);
    if (c == 10) { NL = true; }
  }
}
