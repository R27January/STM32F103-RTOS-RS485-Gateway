# Test Record

This document records the current test results of the STM32F103 FreeRTOS RS485 gateway project.

## Test Environment

| Item          | Description                             |
| ------------- | --------------------------------------- |
| MCU           | STM32F103C8T6                           |
| RTOS          | FreeRTOS                                |
| Communication | UART1 + RS485                           |
| Flash         | W25Q64                                  |
| Display       | OLED                                    |
| PC Tool       | Serial assistant / Python protocol tool |
| Protocol      | Custom frame + CRC16-Modbus             |

## V1: ADC, OLED and UART Baseline Test

### Test Goal

Verify that the MCU can acquire ADC data, display it on OLED, and output data through UART.

### Test Result

| Item            | Result |
| --------------- | ------ |
| ADC acquisition | Passed |
| OLED display    | Passed |
| UART output     | Passed |

### Conclusion

The basic data acquisition and display path is working.

---

## V2: FreeRTOS Queue Data Transfer Test

### Test Goal

Verify that `SensorTask` can send sensor data to `DisplayTask` and `CommTask` through queues.

### Test Result

| Item                 | Result |
| -------------------- | ------ |
| `DisplayDataQueue`   | Passed |
| `CommDataQueue`      | Passed |
| Sensor data transfer | Passed |

### Conclusion

The queue-based data transfer between tasks is working.

---

## V3: Mutex Protection Test

### Test Goal

Verify that shared hardware resources can be protected by mutexes.

### Test Result

| Item                             | Result |
| -------------------------------- | ------ |
| `OLEDMutex`                      | Passed |
| `UARTMutex`                      | Passed |
| OLED resource protection         | Passed |
| UART / RS485 resource protection | Passed |

### Conclusion

The mutex-based shared resource protection is working.

---

## V4: Custom Protocol and CRC16-Modbus Test

### Test Goal

Verify the custom protocol frame parsing and CRC16-Modbus checking.

Protocol frame format:

```text id="etp00k"
AA 55 LEN CMD DATA CRC_L CRC_H
```

### Test Cases

| Test Case                | Input               | Expected Result        | Result |
| ------------------------ | ------------------- | ---------------------- | ------ |
| Valid current data query | `AA 55 01 01 C1 E0` | Return sensor response | Passed |
| Wrong frame header       | Invalid header      | Return error response  | Passed |
| Wrong length             | Invalid LEN         | Return error response  | Passed |
| Wrong command            | Invalid CMD         | Return error response  | Passed |
| Wrong CRC                | Invalid CRC         | Return CRC error       | Passed |

### Conclusion

The custom protocol and CRC16-Modbus checking are working.

---

## V5: RS485 Half-Duplex Communication Test

### Test Goal

Verify RS485 half-duplex communication between PC and STM32.

### Hardware Connection

```text id="y82747"
USB-RS485 A  →  TTL-RS485 A
USB-RS485 B  →  TTL-RS485 B
GND          →  GND
```

RS485 direction control:

| PB12 Level | Mode     |
| ---------- | -------- |
| 0          | Receive  |
| 1          | Transmit |

### Test Result

| Item                                | Result |
| ----------------------------------- | ------ |
| PC sends RS485 request              | Passed |
| STM32 receives request              | Passed |
| STM32 switches to transmit mode     | Passed |
| STM32 sends response                | Passed |
| STM32 switches back to receive mode | Passed |

### Conclusion

The RS485 half-duplex communication path is working.

---

## V6: W25Q64 Log Storage Test

### Test Goal

Verify W25Q64 external Flash read/write and historical log query.

### Test Result

| Item                                    | Result |
| --------------------------------------- | ------ |
| Read W25Q64 JEDEC ID                    | Passed |
| Sector erase                            | Passed |
| Page program                            | Passed |
| Data readback                           | Passed |
| Save log record                         | Passed |
| Query historical log by sequence number | Passed |

### Example Historical Log Query

Request:

```text id="wvgd2v"
AA 55 05 02 00 00 00 01 B8 4E
```

Meaning:

```text id="fbvxk9"
Query historical log with seq = 1
```

Example response:

```text id="q5rrr7"
AA 55 0B 82 00 00 00 01 01 00 01 01 04 00 60 4E
```

### Conclusion

The W25Q64 log writing and historical log query path is working.

---

## V7: UART DMA + IDLE + Task Notification Test

### Test Goal

Replace blocking UART reception with UART DMA + IDLE reception and wake up `CommTask` by FreeRTOS task notification.

### Reception Flow

```text id="gdwwdr"
UART DMA receives data
↓
UART IDLE event occurs
↓
HAL fixed function records received length
↓
Task notification wakes up CommTask
↓
CommTask parses protocol frame
↓
Restart DMA reception
```

### Test Result

| Item                                    | Result |
| --------------------------------------- | ------ |
| UART DMA reception                      | Passed |
| IDLE event detection                    | Passed |
| `HAL_UARTEx_RxEventCallback` execution  | Passed |
| `CommTask` wake-up by task notification | Passed |
| Current data query                      | Passed |
| Historical log query                    | Passed |
| Continuous frame communication          | Passed |

### Example Current Data Query

Request:

```text id="kzxcjr"
AA 55 01 01 C1 E0
```

Example response:

```text id="v7gyr1"
AA 55 05 81 00 05 0B 90 2B 0D
```

### Conclusion

The UART DMA + IDLE + task notification reception mechanism is working.

---

## V8: Event Group and Software Timer Test

### Test Goal

Add communication status management by event group and communication timeout recovery by software timer.

### Event Bits

| Event Bit            | Meaning                      |
| -------------------- | ---------------------------- |
| `EVENT_COMM_OK`      | Valid communication received |
| `EVENT_PROTOCOL_ERR` | Protocol error occurred      |

### Software Timer

| Timer              | Function                                                              |
| ------------------ | --------------------------------------------------------------------- |
| `CommTimeoutTimer` | Clear `EVENT_COMM_OK` after 3 seconds without new valid communication |

### Test Cases

| Test Case                                | Expected Result                        | Result |
| ---------------------------------------- | -------------------------------------- | ------ |
| Power on                                 | OLED displays `IDLE`                   | Passed |
| Send valid current data query            | OLED displays `COMM OK`                | Passed |
| No new valid communication for 3 seconds | OLED returns to `IDLE`                 | Passed |
| Send valid historical log query          | OLED displays `COMM OK`                | Passed |
| Send CRC error frame                     | OLED displays `PROTO ERR`              | Passed |
| Send valid frame after error             | OLED recovers to `COMM OK`             | Passed |
| Send valid frames continuously           | OLED keeps displaying `COMM OK`        | Passed |
| Stop sending valid frames                | OLED returns to `IDLE` after 3 seconds | Passed |

### Conclusion

The event group based communication status management and software timer based timeout recovery are working.

---

## Current Baseline Conclusion

The current baseline version has completed and passed tests for:

* ADC acquisition
* OLED display
* FreeRTOS queues
* FreeRTOS mutexes
* Custom RS485 protocol
* CRC16-Modbus verification
* RS485 half-duplex communication
* W25Q64 historical log storage
* UART DMA + IDLE reception
* FreeRTOS task notification
* FreeRTOS event group
* FreeRTOS software timer

The project is now ready for the next stage:

```text id="hgj6gx"
V9: Log management enhancement
```

Planned V9 test targets:

* Query device information
* Query log information
* Clear log area
* Manage log count and latest sequence
* Improve log validity checking
