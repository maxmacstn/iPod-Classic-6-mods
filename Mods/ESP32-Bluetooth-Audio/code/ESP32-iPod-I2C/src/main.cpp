#include <Arduino.h>

#include "Wire.h"

#define I2C_PIN_SCL         21
#define I2C_PIN_SDA        22
// #define LED_PIN 23
// #define WAKE 21
// #define BTN_IN 22

#define I2C_DEV_ADDR 0x4A
#include "driver/periph_ctrl.h"

uint32_t i = 0;

void setup() {
  // pinMode(LED_PIN, OUTPUT);

  // for (int i = 0 ; i < 10; i++){
  //   digitalWrite(LED_PIN, i%2);
  //   delay(200);
  // }

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Wire.setClock(400000);
  Wire.setPins(I2C_PIN_SDA,I2C_PIN_SCL);
  // periph_module_disable(PERIPH_I2C0_MODULE);

  Wire.begin();

}

void loop() {
  delay(5000);

  // Write message to the slave
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.write(0x04);
  uint8_t error = Wire.endTransmission(true);
  Serial.printf("Send Write: %u\n", error);

  uint8_t bytesReceived =  Wire.requestFrom(I2C_DEV_ADDR,1,true);

  Serial.printf("requestFrom: %u\n", bytesReceived);
  if ((bool)bytesReceived) {  //If received more than zero bytes
    uint8_t temp[bytesReceived];
    Wire.readBytes(temp, bytesReceived);
    log_print_buf(temp, bytesReceived);
  }
  delay(5000);

  // Set
  Wire.beginTransmission(I2C_DEV_ADDR);
  Wire.write(0x04);
  Wire.write(0x2A);
  error = Wire.endTransmission(true);
  Serial.printf("Send Write: %u\n", error);

}