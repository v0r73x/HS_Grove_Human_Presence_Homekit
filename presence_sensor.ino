#include <Wire.h>
#include "HomeSpan.h"  
#include "Grove_Human_Presence_Sensor.h"

// Instantiate the sensor
AK9753 movementSensor;

// Define sensitivity and detection interval
float sensitivity_presence = 6.0;
float sensitivity_movement = 10.0;
int detect_interval = 30; // milliseconds
PresenceDetector detector(movementSensor, sensitivity_presence, sensitivity_movement, detect_interval);

uint32_t last_time;

// Create global pointers for the services and characteristics
Service::OccupancySensor* occupancyService;
Characteristic::OccupancyDetected* occupancyDetected;

void setup() {
    Serial.begin(115200);
    Serial.println("Grove - Human Presence Sensor example");

    Wire.begin();

    // Initialize the movement sensor
    if (movementSensor.initialize() == false) {
        Serial.println("Device not found. Check wiring.");
        while (1); // Halt if device isn't found
    }

    // Initialize HomeSpan
    homeSpan.setWifiCredentials("SSID", "XXXXXXXXXXXXXXXXXXXXXXXXXXX"); // Change to your WiFi credentials
    homeSpan.setPairingCode("09654111"); // HomeKit pairing code
    homeSpan.setControlPin(0);
    homeSpan.enableOTA(false); // Disable OTA updates
    homeSpan.begin(Category::Sensors, "HS Presence");

    // Define the HomeKit accessory and services
    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();
    new Characteristic::Name("Presence Sensor");

    // Occupancy sensor service
    occupancyService = new Service::OccupancySensor();  
    occupancyDetected = new Characteristic::OccupancyDetected();

    last_time = millis();  // Initialize last_time
}

void loop() {
    uint8_t presenceStatus = 0;

    // Check if any field is detecting presence (if any field is 1, set presenceStatus to 1)
    presenceStatus = (detector.presentField1() || detector.presentField2() || detector.presentField3() || detector.presentField4()) ? 1 : 0;

    // Update the OccupancyDetected characteristic value (set to 0 or 1)
    occupancyDetected->setVal(presenceStatus);

    // Poll HomeSpan to handle communication
    homeSpan.poll();

    // Process sensor data in the detection loop
    detector.loop();

    uint32_t now = millis();
    if (now - last_time > 100) {
        // Print the presence status as a uint8_t value (in decimal)
        Serial.print(presenceStatus, DEC);  // Print as decimal value
        Serial.print(" ");  // Print a space for readability
        
        last_time = now;  // Update last_time
    }
}
