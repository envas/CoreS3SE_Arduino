/*
 * Example for the "CWT-THxxS Series RS485 temperature and humidity sensor" with Modbus address = 2
 */

#include <Arduino.h>
#include "ModbusClientRTU.h"
#include "Logging.h"

#include "../../M5StamPLC/include/main.h"


bool     data_ready = false;
uint16_t humidity;
int16_t  temperature;
uint32_t request_time;

vector<uint8_t> data = {};

// Create a ModbusRTU client instance
// The RS485 module has no halfduplex, so the parameter with the DE/RE pin is required!
ModbusClientRTU MB(REDEPIN);

// Define an onData handler function to receive the regular responses
// Arguments are received response message and the request's token
void handleData(ModbusMessage response, uint32_t token) {
    // The first value is on pos 3, after server ID, function code and length byte

    response.get(3, data, 4);
    humidity    = data[0] << 8 | data[1];
    temperature = (data[2] << 8 | data[3]);

    // Signal "data is complete"
    request_time = token;
    data_ready   = true;
}

// Define an onError handler function to receive error responses
// Arguments are the error code returned and a user-supplied token to identify the causing request
void handleError(Error error, uint32_t token) {
    // ModbusError wraps the error code and provides a readable error message for it
    ModbusError me(error);
    LOG_E("Error response: %02X - %s\n", (int)me, (const char *)me);
}

// Setup() - initialization happens here
void setup() {
    // Init Serial monitor
    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("__ OK __");

    // Set up Serial2 connected to Modbus RTU
    RTUutils::prepareHardwareSerial(Serial2);
    Serial2.begin(BAUDRATE, SERIAL_8N1, RXPIN, TXPIN);

    // Set up ModbusRTU client.
    // - provide onData handler function
    MB.onDataHandler(&handleData);
    // - provide onError handler function
    MB.onErrorHandler(&handleError);
    // Set message timeout to 2000ms
    MB.setTimeout(2000);
    // Start ModbusRTU background task
    MB.begin(Serial2);
}

// loop() - cyclically request the data
void loop() {
    static unsigned long next_request = millis();

    // Shall we do another request?
    if (millis() - next_request > READ_INTERVAL) {
        // Yes.
        data_ready = false;
        // Issue the request
        Error err = MB.addRequest((uint32_t)millis(), 2, READ_HOLD_REGISTER, FIRST_REGISTER, NUM_VALUES);
        if (err != SUCCESS) {
            ModbusError e(err);
            LOG_E("Error creating request: %02X - %s\n", (int)e, (const char *)e);
        }
        // Save current time to check for next cycle
        next_request = millis();
    } else {
        // No, but we may have another response
        if (data_ready) {
            // We do. Print out the data
            int16_t temp = (int16_t)temperature;
            Serial.printf("Requested at %8.3fs:\n", request_time / 1000.0);
            Serial.printf("   humidity   : %3.1f\n", humidity / 10.0f);
            Serial.printf("   temperature: %3.1f\n", temperature / 10.0f);

            Serial.printf("----------\n\n");
            data_ready = false;
        }
    }
}
