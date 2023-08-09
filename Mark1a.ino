/*
Alfred Liang 07/08/2023
This Program is for M-Duino 21+ PLC setting as master device
to communicate via RS485 Modbus RTU for Mark Ia reactor

In the main loop there are two functions: 
Read registers for temperature and Write registers for power
*/

#include <LiquidCrystal.h>
#include <ModbusRTUMaster.h>
#include <RS485.h>

ModbusRTUMaster master(RS485);
uint32_t lastSentTime = 0UL;
uint32_t lastTime = 0UL;
const uint32_t baudrate = 19200UL; 
uint16_t temperature[4] = {70,70,70,70};
uint16_t input[4] = {50,50,50,50};
int flag = 0;
int cycle = 1;
int maxCycle = 25;
int power = 50;


//////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(19200UL); // Start the serial port
  RS485.begin(baudrate, HALFDUPLEX, SERIAL_8E1); // SERIAL_8E1: 8 data bit, Even parity, 1 stop bit
  master.begin(baudrate);
}

void loop() {

  // read temperature
  if (millis() - lastSentTime > 2000) {
      // (readHoldingRegisters(slave add, start add, number of reg))
      // start add: 0 - power, 18 - state, 36 - temp
    if (!master.readHoldingRegisters(1, 36, 4)) {
      Serial.print("Read registers failed ");
      flag = 0;
      Serial.print(millis());
      Serial.print(" ");
      Serial.println(lastSentTime);
    } else {
      flag = 3;
    }

    lastSentTime = millis();
  }

  if (master.isWaitingResponse() && flag == 3) {
    ModbusResponse response = master.available();
    if (response) {
      if (response.hasError()) {
        // Response failure treatment. You can use response.getErrorCode()
        // to get the error code.
        Serial.print("Read Error ");
        Serial.println(response.getErrorCode());
      } else {
        // Get the discrete inputs values from the response
        if (response.hasError()) {
          // Response failure treatment. You can use response.getErrorCode()
          // to get the error code.
          Serial.print("Read Error ");
          Serial.println(response.getErrorCode());
        } else {
          Serial.print("Temperature status: ");
          for (int i = 0; i < 4; ++i) {
            int res = response.getRegister(i);
            Serial.print(res);
            Serial.print(',');
          }
          Serial.println();

        }
      }
    }
  }

  // check temperature
  for (int k = 0; k < 4; ++k) {
    if (temperature[k] > 95) {
      Serial.print(k);
      Serial.print(" PSU error:");
      Serial.print(temperature[k]);
      Serial.println();
    }
  }
  
  // Power scheme
  // 1 min ON, 1.5 min OFF
  if (cycle > maxCycle) {
    power = 0;
  } else if ((millis()-lastTime) < 60000) {
    power = 66
  } else if ((millis()-lastTime) >= 60000 && (millis()-lastTime) < 150000) {
    power = 0; // 1.5 min OFF 
  } else {
    lastTime = millis();
    cycle++;
  }

  for (int i = 0; i < 4; i++) {
    input[i] = power; 
  }

  // write power to PSUs
  if (millis() - lastSentTime > 2000) {
      // start add: 0 - power, 18 - state, 37 - temp
      if (!master.writeMultipleRegisters(1, 0, input, 4)) {
        Serial.print("Write registers failed ");
        Serial.print(millis());
        Serial.print(" ");
        Serial.println(lastSentTime);
        flag = 0;
      } else {
        flag = 2;
      }

    lastSentTime = millis();
  }

  if (master.isWaitingResponse() && flag == 2) {
    ModbusResponse response = master.available();
    if (response) {
      if (response.hasError()) {
        // Response failure treatment. You can use response.getErrorCode()
        // to get the error code.
        Serial.print("Write Error ");
        Serial.println(response.getErrorCode());

        lcd.setCursor(0, 1);
        lcd.print("Write Error");
        lcd.print("              ");
      } else {
        // Get the discrete inputs values from the response
        if (response.hasError()) {
          // Response failure treatment. You can use response.getErrorCode()
          // to get the error code.
          Serial.print("Write Error ");
          Serial.println(response.getErrorCode());
        } else {
          Serial.print("Write Done: ");
          lcd.setCursor(0, 1);
          lcd.print("PWR:");
          for (int i = 0; i < 4; ++i) {
            Serial.print(input[i]/1.32,0);
            Serial.print('%,');

            lcd.print(input[i]/1.32,0);
            lcd.print(" ");
          }
          lcd.print("              ");
          Serial.println();
        }
      }
    }
  }

  // print current cycle and time
  if (millis() % 5000 == 0) {
    Serial.print(cycle);
    if (cycle > maxCycle) {
      Serial.print(maxCycle);
      Serial.print("/");
      Serial.print(maxCycle);
      Serial.println(" Finished");
    } else {
      Serial.print(cycle);
      Serial.print("/");
      Serial.print(maxCycle);
      Serial.print(" ");
    }
    
    uint32_t timeBegin = millis()/60000;
    Serial.print(timeBegin);
    Serial.println("min begin");
  }
}
