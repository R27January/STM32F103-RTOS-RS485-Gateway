# Communication Protocol

This document describes the current custom RS485 communication protocol used by this project.

The current protocol is used between the PC-side tool and the STM32 gateway terminal.

## Frame Format

The protocol frame format is:

```text
AA 55 LEN CMD DATA CRC_L CRC_H
```

| Field   |     Size | Description            |
| ------- | -------: | ---------------------- |
| `AA 55` |  2 bytes | Frame header           |
| `LEN`   |   1 byte | Length of `CMD + DATA` |
| `CMD`   |   1 byte | Command code           |
| `DATA`  | Variable | Command payload        |
| `CRC_L` |   1 byte | CRC16-Modbus low byte  |
| `CRC_H` |   1 byte | CRC16-Modbus high byte |

Total frame length:

```text
Total Length = LEN + 5
```

Because:

```text
2 bytes header + 1 byte LEN + LEN bytes CMD/DATA + 2 bytes CRC
```

## CRC Rule

The protocol uses CRC16-Modbus.

CRC parameters:

| Item          | Value          |
| ------------- | -------------- |
| Algorithm     | CRC16-Modbus   |
| Polynomial    | `0xA001`       |
| Initial value | `0xFFFF`       |
| Output order  | Low byte first |

CRC calculation range:

```text
LEN + CMD + DATA
```

The frame header `AA 55` and the CRC bytes themselves are not included in the CRC calculation.

## Current Commands

### 1. Query Current Sensor Data

Command:

```text
CMD_GET_SENSOR = 0x01
```

Request frame:

```text
AA 55 01 01 C1 E0
```

Meaning:

| Field  | Value   | Description                  |
| ------ | ------- | ---------------------------- |
| Header | `AA 55` | Frame header                 |
| LEN    | `01`    | CMD length only              |
| CMD    | `01`    | Query current sensor data    |
| CRC    | `C1 E0` | CRC16-Modbus, low byte first |

Response command:

```text
CMD_RESP_SENSOR = 0x81
```

Response frame format:

```text
AA 55 05 81 CNT_H CNT_L ADC_H ADC_L CRC_L CRC_H
```

Response data:

| Field         |    Size | Description       |
| ------------- | ------: | ----------------- |
| `CNT_H CNT_L` | 2 bytes | Sample counter    |
| `ADC_H ADC_L` | 2 bytes | Current ADC value |

Example response:

```text
AA 55 05 81 00 05 0B 90 2B 0D
```

---

### 2. Query Historical Log

Command:

```text
CMD_GET_LOG = 0x02
```

Request frame format:

```text
AA 55 05 02 SEQ_3 SEQ_2 SEQ_1 SEQ_0 CRC_L CRC_H
```

Request data:

| Field                     |    Size | Description         |
| ------------------------- | ------: | ------------------- |
| `SEQ_3 SEQ_2 SEQ_1 SEQ_0` | 4 bytes | Log sequence number |

Response command:

```text
CMD_RESP_LOG = 0x82
```

Response frame format:

```text
AA 55 0B 82 SEQ_3 SEQ_2 SEQ_1 SEQ_0 TYPE CNT_H CNT_L ADC_H ADC_L ERR_CODE CRC_L CRC_H
```

Response data:

| Field                     |    Size | Description         |
| ------------------------- | ------: | ------------------- |
| `SEQ_3 SEQ_2 SEQ_1 SEQ_0` | 4 bytes | Log sequence number |
| `TYPE`                    |  1 byte | Log type            |
| `CNT_H CNT_L`             | 2 bytes | Sample counter      |
| `ADC_H ADC_L`             | 2 bytes | ADC value           |
| `ERR_CODE`                |  1 byte | Error code          |

Example request:

```text
AA 55 05 02 00 00 00 01 B8 4E
```

Meaning:

```text
Query historical log with seq = 1
```

Example response:

```text
AA 55 0B 82 00 00 00 01 01 00 01 01 04 00 60 4E
```

---

## Error Response

Error response command:

```text
CMD_ERROR = 0xE0
```

Error response frame format:

```text
AA 55 02 E0 ERR_CODE CRC_L CRC_H
```

Error code examples:

| Error Code | Description        |
| ---------- | ------------------ |
| `0x01`     | Frame header error |
| `0x02`     | Length error       |
| `0x03`     | Command error      |
| `0x04`     | CRC error          |

## Planned Commands

The following commands will be added in future versions.

### 1. Query Device Information

Planned command:

```text
CMD_GET_DEVICE_INFO
```

Planned response data:

| Field            | Description              |
| ---------------- | ------------------------ |
| Device ID        | Unique device number     |
| Firmware version | Current firmware version |
| Protocol version | Current protocol version |
| Log capacity     | Maximum log record count |

---

### 2. Query Log Information

Planned command:

```text
CMD_GET_LOG_INFO
```

Planned response data:

| Field             | Description                    |
| ----------------- | ------------------------------ |
| Log count         | Current valid log record count |
| Latest sequence   | Latest log sequence number     |
| Maximum log count | Maximum log capacity           |

---

### 3. Clear Log Area

Planned command:

```text
CMD_CLEAR_LOG
```

Planned behavior:

```text
Erase W25Q64 log area
Reset log count
Reset latest sequence
Return clear success response
```

---

### 4. Query Communication Diagnostics

Planned command:

```text
CMD_GET_DIAG_INFO
```

Planned response data:

| Field               | Description                      |
| ------------------- | -------------------------------- |
| Total request count | Number of received requests      |
| Valid request count | Number of valid requests         |
| CRC error count     | Number of CRC errors             |
| Timeout count       | Number of communication timeouts |
| Latest error code   | Most recent error code           |

## Notes

The current protocol is a custom protocol designed for project learning and testing.

Future versions may add Modbus RTU support or provide a Modbus-style register mapping.
## CMD_GET_DEVICE_INFO

| Item          | Value                           |
| ------------- | ------------------------------- |
| Command       | `CMD_GET_DEVICE_INFO`           |
| Command Code  | `0x03`                          |
| Response      | `CMD_RESP_DEVICE_INFO`          |
| Response Code | `0x83`                          |
| Direction     | PC / Python -> Gateway          |
| Purpose       | Query basic gateway information |

### Request Frame

```text
AA 55 01 03 CRC_L CRC_H
```

Example:

```text
AA 55 01 03 40 21
```

### Response Frame

```text
AA 55 0B 83 DATA CRC_L CRC_H
```

### Response DATA Format

| Field              |    Size | Description                      |
| ------------------ | ------: | -------------------------------- |
| `gateway_id`       | 2 bytes | Gateway device ID                |
| `fw_version`       | 2 bytes | Firmware version                 |
| `protocol_version` | 2 bytes | Upstream custom protocol version |
| `feature_flags`    | 2 bytes | Supported feature bitmap         |
| `device_status`    | 2 bytes | Basic device running status      |

### Byte Order

All 16-bit data fields are transmitted in high-byte-first order.

CRC16-Modbus is transmitted in low-byte-first order:

```text
CRC_L CRC_H
```

### CRC Range

CRC16-Modbus is calculated over:

```text
LEN + CMD + DATA
```

Frame header `AA 55` and CRC bytes are not included in the CRC calculation.
## V9.2 Log Information Query

### Command: Get Log Runtime Information

This command is used by the host PC to query the current runtime status of the log system.

Request frame:

```text
AA 55 01 04 01 E3
```

Frame format:

| Field | Value | Description                  |
| ----- | ----: | ---------------------------- |
| HEAD  | AA 55 | Frame header                 |
| LEN   |    01 | CMD only, no DATA            |
| CMD   |    04 | CMD_GET_LOG_INFO             |
| CRC   | 01 E3 | CRC16-Modbus, low byte first |

### Response Frame

Response command:

```text
CMD_RESP_LOG_INFO = 0x84
```

Response LEN:

```text
PROTOCOL_LEN_RESP_LOG_INFO = 0x10
```

Response total frame length:

```text
21 bytes
```

Response format:

```text
AA 55 10 84 LOG_COUNT LATEST_SEQ MAX_COUNT RECORD_SIZE LOG_FULL CRC_L CRC_H
```

DATA field layout:

| Field       |    Size | Description                         |
| ----------- | ------: | ----------------------------------- |
| log_count   | 4 bytes | Current runtime log count           |
| latest_seq  | 4 bytes | Latest accepted log sequence number |
| max_count   | 4 bytes | Maximum supported log record count  |
| record_size | 2 bytes | Size of one `LogData_t` record      |
| log_full    |  1 byte | 0 = not full, 1 = full              |

Byte order:

```text
Normal multi-byte DATA fields: high byte first
CRC16 field: low byte first
```

Example response after power-on, before sensor query:

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

Example response after two successful sensor queries:

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
