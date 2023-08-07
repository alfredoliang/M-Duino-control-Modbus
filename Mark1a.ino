/*
Alfred Liang 07/08/2023
This Program is for M-Duino 21+ PLC setting as master device
to communicate via RS485 Modbus RTU for Mark Ia reactor
Turn ON 2 min and OFF 1 min continue for 24 cycles
*/

#include <LiquidCrystal.h>
#include <ModbusRTUMaster.h>
#include <RS485.h>

LiquidCrystal lcd(Q0_7, Q0_6, Q0_3, Q0_2, Q0_1, Q0_0);

ModbusRTUMaster master(RS485);
uint32_t lastSentTime = 0UL;
uint32_t lastTime = 0UL;
const uint32_t baudrate = 19200UL; 
unsigned long time[4] = {millis(),millis(),millis(),millis()};
uint16_t commStat[4] = {1,1,1,1};
uint16_t input[4] = {50,50,50,50};
int flag = 0;
int cycle = 1;
int maxCycle = 25;
int power = 50;


//////////////////////////////////////////////////////////////////
void setup() {
  delay(1000);
  Serial.begin(19200UL); // Start the serial port
  RS485.begin(baudrate, HALFDUPLEX, SERIAL_8E1); // SERIAL_8E1: 8 data bit, Even parity, 1 stop bit
  master.begin(baudrate);

  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Wait for power");
  
  // Read communication status, if PSU all on power, start the program
  while (power != 0) {
    if (millis() - lastSentTime > 1000) {
        // (readHoldingRegisters(slave add, start add, number of reg))
        // start add: 0 - power, 18 - state, 37 - temp
      if (!master.readHoldingRegisters(1, 18, 4)) {
        Serial.print("Read registers failed ");
        flag = 0;
        Serial.print(millis());
        Serial.print(" ");
        Serial.println(lastSentTime);
        lcd.setCursor(0, 1);
        lcd.print("Read failed");
      } else {
        flag = 1;
      }

      lastSentTime = millis();
    }

    // Check available responses often
    if (master.isWaitingResponse() && flag == 1) {
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
            Serial.print("Communication status: ");
            //lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print("Com:");
            power = 0;
            for (int i = 0; i < 4; ++i) {
              int res = response.getRegister(i);
              Serial.print(res);
              if (res == 0) {
                lcd.print("v ");
              } else {
                lcd.print(res,HEX);
                lcd.print(" ");
              }
              commStat[i] = res;
              Serial.print(',');

              power = power + res;
            }
            Serial.println();

          }
        }
      }
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("Session Begin       ");
  delay(1000);

  lastTime = millis();
  delay(1000);
}

void loop() {

  // read temperature
  if (millis() - lastSentTime > 1000) {
      // (readHoldingRegisters(slave add, start add, number of reg))
      // start add: 0 - power, 18 - state, 36 - temp
    if (!master.readHoldingRegisters(1, 36, 4)) {
      Serial.print("Read registers failed ");
      flag = 0;
      Serial.print(millis());
      Serial.print(" ");
      Serial.println(lastSentTime);

      lcd.setCursor(0, 1);
      lcd.print("Read tem failed");
      lcd.print("              ");
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
          lcd.setCursor(0, 0);
          lcd.print("Temp:");
          for (int i = 0; i < 4; ++i) {
            int res = response.getRegister(i);
            Serial.print(res);
            commStat[i] = res;
            Serial.print(',');
            lcd.print(res);
            lcd.print(" ");
          }
          lcd.print("              ");
          Serial.println();

        }
      }
    }
  }

// check comm status of each PSU every 10s
  if (millis() % 10000 == 0) {
    for (int k = 0; k < 4; ++k) {
      if (commStat[k] != 0) {
        Serial.print(k);
        Serial.print(" PSU error:");
        Serial.print(commStat[k]);
        Serial.println();

        lcd.setCursor(0, 1);
        lcd.print("PSU ");
        lcd.print(k);
        lcd.print(" error:");
        lcd.print(commStat[k]);
        lcd.print("          ");
        input[k] = 0;
      }
    }
  }

  // read comm status
  if (millis() - lastSentTime > 1000) {
      // (readHoldingRegisters(slave add, start add, number of reg))
      // start add: 0 - power, 18 - state, 37 - temp
    if (!master.readHoldingRegisters(1, 18, 4)) {
      Serial.print("Read comm failed ");
      flag = 0;
      Serial.print(millis());
      Serial.print(" ");
      Serial.println(lastSentTime);

      lcd.setCursor(0, 1);
      lcd.print("Read comm failed");
      lcd.print("              ");
    } else {
      flag = 1;
    }

    lastSentTime = millis();
  }

  if (master.isWaitingResponse() && flag == 1) {
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
          Serial.print("Communication status: ");
          lcd.setCursor(0, 1);
          lcd.print("Com:");
          for (int i = 0; i < 4; ++i) {
            int res = response.getRegister(i);
            Serial.print(res);
            if (res == 0) {
              lcd.print("v ");
            } else {
              lcd.print(res);
              lcd.print(" ");
            }
            commStat[i] = res;
            Serial.print(',');
          }
          lcd.print("             ");
          Serial.println();
        }
      }
    }
  }

  // Power scheme
  // 1 min ON, 1.5 min OFF, 20%->50% power, 5% increment
  if (cycle > maxCycle) {
    power = 0;
  } else if ((millis()-lastTime) < 60000) {
    power = round(26.4 + (cycle - 1) *6.6);
    if (power > 66) {
      power = 66;
    }
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
  if (millis() - lastSentTime > 1000) {
      // start add: 0 - power, 18 - state, 37 - temp
      if (!master.writeMultipleRegisters(1, 0, input, 4)) {
        Serial.print("Write registers failed ");
        Serial.print(millis());
        Serial.print(" ");
        Serial.println(lastSentTime);
        flag = 0;

        lcd.setCursor(0, 1);
        lcd.print("Write failed");

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
  
  if (millis() % 5000 == 0) {
    Serial.println(cycle);
    if (cycle > maxCycle) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(maxCycle);
      lcd.print("/");
      lcd.print(maxCycle);
      lcd.print(" Finished");
    } else {
      lcd.setCursor(0, 0);
      lcd.print(cycle);
      lcd.print("/");
      lcd.print(maxCycle);
      lcd.print(" ");
    }

    //int timeRemain = 12*6 - millis()/60000;
    int timeBegin = millis()/60000;
    lcd.print(timeBegin);
    lcd.print("min begin");
    lcd.print("              ");
  }
}
