# STM32F103-RTOS-RS485-Gateway

Industrial RS485 data acquisition terminal / gateway prototype based on **STM32F103C8T6** and **FreeRTOS**.

This project is designed as an embedded system practice project for industrial data acquisition, RS485 communication, local data logging, and RTOS-based task scheduling.

## Project Status

Current baseline version completed:

* ADC data acquisition
* OLED status display
* Custom RS485 protocol
* CRC16-Modbus frame checking
* W25Q64 external Flash data logging
* UART DMA + IDLE interrupt reception
* FreeRTOS task notification
* FreeRTOS queues and mutexes
* Event group based communication status management
* Software timer based communication timeout handling

Planned upgrades:

* Log management commands
* Device information query
* Log information query
* Log clear command
* Modbus RTU master polling
* Downstream industrial sensor/device access
* Communication diagnostics and error statistics

## Hardware Platform

* MCU: STM32F103C8T6
* RTOS: FreeRTOS
* Communication: UART / RS485
* Storage: W25Q64 external SPI Flash
* Display: 0.96-inch OLED
* Development tools: STM32CubeMX, Keil MDK, VSCode

## System Architecture

The system is divided into several FreeRTOS tasks:

* `SensorTask`: periodically acquires ADC data
* `DisplayTask`: displays sensor data and communication status on OLED
* `CommTask`: handles RS485 protocol communication
* `LogTask`: writes log data into W25Q64 external Flash

Main RTOS mechanisms used in this project:

* Queue: transfers sensor data between tasks
* Mutex: protects OLED and UART/RS485 resources
* Task notification: wakes up `CommTask` after UART DMA + IDLE reception
* Event group: manages communication status such as `COMM_OK` and `PROTOCOL_ERR`
* Software timer: clears communication status after timeout

## Communication Protocol

The current version uses a custom protocol frame:

```text
AA 55 LEN CMD DATA CRC_L CRC_H
```

Frame fields:

* `AA 55`: frame header
* `LEN`: length of CMD + DATA
* `CMD`: command code
* `DATA`: command payload
* `CRC_L / CRC_H`: CRC16-Modbus checksum, low byte first

Supported commands in the current version:

* Query current ADC data
* Query historical log data

Planned commands:

* Query device information
* Query log information
* Clear log area
* Query communication diagnostics

## Data Logging

The project uses W25Q64 external Flash to store historical log records.

Each log record contains:

* Log sequence number
* Log type
* Sample counter
* ADC value
* Error code

The current version supports writing logs and querying logs by sequence number.

Future versions will add:

* Valid log count
* Latest log sequence
* Log clear command
* Circular log storage
* Log record CRC / valid flag

## Roadmap

### V9: Log Management Enhancement

* Add `CMD_GET_DEVICE_INFO`
* Add `CMD_GET_LOG_INFO`
* Add `CMD_CLEAR_LOG`
* Add log count and latest sequence management

### V10: Modbus RTU Downstream Device Access

* Add Modbus RTU master function
* Poll real RS485 industrial sensors or modules
* Parse Modbus register data

### V11: Gateway Architecture Upgrade

* Separate upstream PC communication and downstream Modbus polling
* Add data cache between communication and acquisition tasks

### V12: Reliability and Diagnostics

* Add device online/offline status
* Add communication timeout retry
* Add CRC error counter
* Add recent error code record
* Add watchdog support

## Repository Structure

```text
firmware/   STM32 firmware source code
docs/       Project documents
images/     Project images, wiring diagrams and test screenshots
tools/      PC-side tools and protocol test scripts
```

## Notes

This project is still under active development.
The current version is a stable baseline version, and future versions will focus on industrial Modbus device access, log management, and communication reliability.
