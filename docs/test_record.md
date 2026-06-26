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
## V9.1 Device Info Query Test

### Purpose

Verify that the gateway supports the new `CMD_GET_DEVICE_INFO` command and can return basic gateway information through the upstream RS485 custom protocol.

### Test Environment

| Item                    | Value                             |
| ----------------------- | --------------------------------- |
| MCU                     | STM32F103C8T6                     |
| RTOS                    | FreeRTOS                          |
| Upstream Interface      | USART1 + RS485                    |
| RS485 Direction Control | PB12                              |
| Protocol                | Custom protocol with CRC16-Modbus |
| Firmware Stage          | V9.1                              |

### Request Frame

```text
AA 55 01 03 40 21
```

### Expected Response

```text
AA 55 0B 83 ...
```

The response should contain the following fields:

| Field              | Expected Meaning          |
| ------------------ | ------------------------- |
| `gateway_id`       | Gateway device ID         |
| `fw_version`       | Firmware version          |
| `protocol_version` | Upstream protocol version |
| `feature_flags`    | Supported feature bitmap  |
| `device_status`    | Basic running status      |

### Test Result

PASS

The gateway returned a valid `CMD_RESP_DEVICE_INFO` response frame. The frame header, length, response command, data fields, and CRC format matched the protocol design.

### Regression Test

| Test Item                    | Result |
| ---------------------------- | ------ |
| `CMD_GET_SENSOR`             | PASS   |
| `CMD_GET_LOG`                | PASS   |
| Invalid frame error response | PASS   |

### Conclusion

V9.1 device information query function passed. The new command did not break the existing V8 protocol functions.
## V9.2 Log Info Query Test

### Test Target

Verify the `CMD_GET_LOG_INFO` command and runtime log status tracking.

This test checks:

```text
1. LogRuntimeInfo initialization
2. log_count update after successful sensor query
3. latest_seq update after successful sensor query
4. CMD_GET_LOG_INFO response frame
5. Repeated CMD_GET_LOG_INFO does not create new logs
6. Existing CMD_GET_LOG query remains valid
```

### Test 1: Query LOG_INFO after power-on

Request:

```text
AA 55 01 04 01 E3
```

Response:

```text
AA 55 10 84 00 00 00 00 00 00 00 00 00 00 00 80 00 0C 00 32 16
```

Decoded result:

```text
log_count   = 0
latest_seq  = 0
max_count   = 128
record_size = 12
log_full    = 0
```

Result:

```text
PASS
```

### Test 2: Query sensor once, then query LOG_INFO

Sensor query:

```text
AA 55 01 01 C1 E0
```

Sensor response:

```text
AA 55 05 81 00 07 0C F9 48 D3
```

LOG_INFO query:

```text
AA 55 01 04 01 E3
```

LOG_INFO response:

```text
AA 55 10 84 00 00 00 01 00 00 00 00 00 00 00 80 00 0C 00 CF D5
```

Decoded result:

```text
log_count   = 1
latest_seq  = 0
max_count   = 128
record_size = 12
log_full    = 0
```

Result:

```text
PASS
```

### Test 3: Query sensor twice, then query LOG_INFO

Sensor query:

```text
AA 55 01 01 C1 E0
```

Sensor response:

```text
AA 55 05 81 00 7D 0C C6 29 1A
```

LOG_INFO response:

```text
AA 55 10 84 00 00 00 02 00 00 00 01 00 00 00 80 00 0C 00 0A 1D
```

Decoded result:

```text
log_count   = 2
latest_seq  = 1
max_count   = 128
record_size = 12
log_full    = 0
```

Result:

```text
PASS
```

### Test 4: Repeated LOG_INFO query

Repeated `CMD_GET_LOG_INFO` requests return the same result:

```text
log_count   = 2
latest_seq  = 1
```

No new log is created by `CMD_GET_LOG_INFO`.

Result:

```text
PASS
```

### Test 5: Existing CMD_GET_LOG regression

Query `seq=0` and `seq=1` using existing `CMD_GET_LOG`.

Result:

```text
Both seq=0 and seq=1 can be queried correctly.
Existing W25Q64 log query function remains valid.
```

Final result:

```text
V9.2 PASS
```
## V9.3 Log Record Validation and Runtime Rebuild Test

### Test Target

Verify the V9.3 log system upgrade, including log record validity checking, per-record CRC protection, and runtime log information rebuild after reset.

### Updated Log Record

The log record structure was upgraded with the following fields:

* `magic`: fixed log marker, used to identify valid V9.3 log records
* `source`: log source, such as local, upstream, or downstream
* `target_id`: target device ID, reserved for downstream Modbus devices
* `type`: log type
* `err_code`: error code
* `seq`: log sequence number
* `cnt`: sensor sample count
* `adc`: ADC value
* `record_crc`: CRC16 checksum for one flash log record

### Test Commands

Query device information:

```text
AA 55 01 03 40 21
```

Query log runtime information:

```text
AA 55 01 04 01 E3
```

Query current sensor data and generate one log record:

```text
AA 55 01 01 C1 E0
```

Query log record `seq=0`:

```text
AA 55 05 02 00 00 00 00 79 8E
```

Query log record `seq=1`:

```text
AA 55 05 02 00 00 00 01 B8 4E
```

Query log record `seq=2`:

```text
AA 55 05 02 00 00 00 02 F8 4F
```

Query unwritten log record `seq=3`:

```text
AA 55 05 02 00 00 00 03 39 8F
```

Query out-of-range log record `seq=128`:

```text
AA 55 05 02 00 00 00 80 78 2E
```

### Test Result

The device information query returned a valid `CMD_RESP_DEVICE_INFO` response.

The initial log information response showed:

```text
log_count   = 0
latest_seq  = 0
max_count   = 128
record_size = 20
log_full    = 0
```

After sending `CMD_GET_SENSOR` three times, three valid log records were generated. The log information response showed:

```text
log_count   = 3
latest_seq  = 2
max_count   = 128
record_size = 20
log_full    = 0
```

The queries for `seq=0`, `seq=1`, and `seq=2` all returned valid `CMD_RESP_LOG` responses.

The query for unwritten `seq=3` returned:

```text
AA 55 02 E0 05 59 C3
```

The query for out-of-range `seq=128` also returned:

```text
AA 55 02 E0 05 59 C3
```

After pressing the reset button, the device rebuilt the log runtime information from W25Q64 flash. The log information remained valid, proving that `Log_RebuildRuntimeInfo()` can recover `log_count` and `latest_seq` after reset.

### Conclusion

V9.3 log validation and runtime rebuild passed the board-level verification. Empty flash records, unwritten log slots, and out-of-range sequence numbers are no longer returned as valid logs.
## V9.4 Clear Log Command Test

### Test Target

Verify that the device can clear W25Q64 log records through an upstream RS485 command, without manually changing the boot-time erase macro.

### Test Commands

Generate log records by sending `CMD_GET_SENSOR` several times:

```text
AA 55 01 01 C1 E0
```

Query log runtime information:

```text
AA 55 01 04 01 E3
```

Send clear-log request with confirmation code `CLR!`:

```text
AA 55 05 05 43 4C 52 21 E5 65
```

Query log record `seq=0`:

```text
AA 55 05 02 00 00 00 00 79 8E
```

### Observed Response

The clear-log command returned:

```text
AA 55 02 85 00 B2 90
```

This means:

```text
CMD_RESP_CLEAR_LOG = 0x85
status = 0x00
```

The result indicates that the clear-log command was accepted and executed successfully.

After clearing logs, querying `seq=0` returned:

```text
AA 55 02 E0 05 59 C3
```

This means:

```text
CMD_RESP_ERROR = 0xE0
ERR_CODE_LOG_INVALID = 0x05
```

The result indicates that the previous log record `seq=0` is no longer valid.

The log runtime information was also cleared, and `CMD_GET_LOG_INFO` returned an empty log state.

### Conclusion

V9.4 clear-log command passed the board-level verification.

The device can now erase the W25Q64 log area through an RS485 command, reset runtime log information, and reject old log queries after clearing. The manual `LOG_ACCEPTANCE_ERASE_ON_BOOT` test switch is no longer required for normal log clearing.
## V10 Hardware Test - Downstream CWS91 Modbus RTU

Date: 2026-06-26

### Test Objective

Verify that the STM32F103 gateway can communicate with the downstream CWS91 RS485 temperature and humidity sensor through USART2 and a TTL-to-RS485 module.

### Hardware Setup

| Module                  | Connection                     |
| ----------------------- | ------------------------------ |
| STM32 USART2 TX/RX      | Downstream TTL-to-RS485 module |
| RS485 Direction Control | PB13                           |
| CWS91 Power Supply      | 12V external power supply      |
| Protocol                | Modbus RTU                     |
| Sensor Address          | 0x01                           |

### Test Result

The STM32 successfully generated the Modbus RTU request frame, sent it through the downstream RS485 bus, received the CWS91 response frame, verified the CRC, and parsed temperature and humidity values.

The OLED temporarily displayed live T/H values during V10 validation.

### Conclusion

V10 passed.

The STM32F103 can work as a Modbus RTU master and read real temperature and humidity data from the downstream CWS91 RS485 sensor.

---

## V11 Hardware Test - Gateway Closed-loop Query

Date: 2026-06-26

### Test Objective

Verify that the upstream PC can query cached downstream CWS91 temperature and humidity data through the STM32 gateway.

The expected data path is:

```text
PC / Serial Tool
→ USB-RS485
→ STM32 USART1 upstream custom AA55 protocol
→ DownstreamData cache
→ Cached CWS91 temperature and humidity response
```

The STM32 should return cached downstream data instead of blocking the upstream command while reading the CWS91 sensor.

### Test 1: Query Downstream Cached Data

Sent frame:

```text
AA 55 01 06 80 22
```

Received frame:

```text
AA 55 0B 86 01 01 01 0C 03 53 00 00 00 5C E3 0C
```

Parsed result:

| Field            | Value                  |
| ---------------- | ---------------------- |
| Response Command | 0x86                   |
| target_id        | 0x01                   |
| valid            | 0x01                   |
| temperature      | 0x010C / 10 = 26.8 °C  |
| humidity         | 0x0353 / 10 = 85.1 %RH |
| update_count     | 0x0000005C = 92        |
| CRC              | 0x0CE3, sent as E3 0C  |

Conclusion:

The STM32 successfully returned cached downstream CWS91 data through the upstream AA55 custom protocol.

### Test 2: Original GET_SENSOR Regression Test

Sent frame:

```text
AA 55 01 01 C1 E0
```

Received frame:

```text
AA 55 05 81 00 C5 0D E3 69 74
```

Parsed result:

| Field            | Value         |
| ---------------- | ------------- |
| Response Command | 0x81          |
| sample_count     | 0x00C5 = 197  |
| adc_value        | 0x0DE3 = 3555 |

Conclusion:

The original GET_SENSOR command still works after adding the V11 downstream data cache query.

### Final V11 Conclusion

V11 passed.

The gateway now supports a complete upstream-to-downstream closed loop. The STM32 periodically polls the downstream CWS91 sensor in the background, updates the DownstreamData cache, and responds to upstream PC queries with the latest cached temperature and humidity data.
## V12 Hardware Test - Downstream Diagnostics and Online Status

Date: 2026-06-26

### Test Objective

Verify that the STM32F103 gateway can detect downstream CWS91 online/offline status, record communication errors, and return diagnostic counters through the upstream AA55 custom protocol.

The expected diagnostic functions are:

* Detect downstream online status during normal communication
* Detect offline status after disconnecting downstream A/B lines
* Record timeout count
* Clear last error after communication recovery
* Return diagnostic counters through CMD_GET_DOWNSTREAM_STATUS

### Test Command

Sent frame:

```text
AA 55 01 07 41 E2
```

Expected response command:

```text
CMD_RESP_DOWNSTREAM_STATUS = 0x87
```

---

### Test 1: Normal Online Status

Received frame:

```text
AA 55 14 87 01 01 00 00 00 00 0F 00 00 00 0F 00 00 00 00 00 00 00 00 DF 98
```

Parsed result:

| Field         | Value |
| ------------- | ----- |
| target_id     | 0x01  |
| online        | 0x01  |
| last_err      | 0x00  |
| poll_count    | 15    |
| success_count | 15    |
| timeout_count | 0     |
| crc_err_count | 0     |

Conclusion:

The downstream CWS91 sensor was online. Polling and success counters increased normally, and no timeout or CRC error was detected.

---

### Test 2: Offline Detection After Disconnecting Downstream A/B

Received frame after disconnecting downstream A/B lines:

```text
AA 55 14 87 01 00 01 00 00 00 6D 00 00 00 69 00 00 00 03 00 00 00 00 9E B4
```

Parsed result:

| Field         | Value |
| ------------- | ----- |
| target_id     | 0x01  |
| online        | 0x00  |
| last_err      | 0x01  |
| poll_count    | 109   |
| success_count | 105   |
| timeout_count | 3     |
| crc_err_count | 0     |

Conclusion:

After the downstream A/B lines were disconnected, the gateway detected communication timeout and marked the downstream device as offline.

---

### Test 3: Recovery After Reconnecting Downstream A/B

Received frame after reconnecting downstream A/B lines:

```text
AA 55 14 87 01 01 00 00 00 00 4C 00 00 00 04 00 00 00 48 00 00 00 00 B7 A5
```

Parsed result:

| Field         | Value |
| ------------- | ----- |
| target_id     | 0x01  |
| online        | 0x01  |
| last_err      | 0x00  |
| poll_count    | 76    |
| success_count | 4     |
| timeout_count | 72    |
| crc_err_count | 0     |

Conclusion:

After the downstream A/B lines were reconnected correctly, the gateway recovered communication with the CWS91 sensor. The online flag returned to 1, and the last error code was cleared.

### Final V12 Conclusion

V12 passed.

The gateway now supports downstream online/offline detection, timeout statistics, CRC error statistics, and upstream diagnostic query through CMD_GET_DOWNSTREAM_STATUS.
