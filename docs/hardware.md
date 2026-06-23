# Hardware Design

This document describes the hardware platform and peripheral connections of this project.

## Hardware Platform

| Item                    | Description                        |
| ----------------------- | ---------------------------------- |
| MCU                     | STM32F103C8T6                      |
| Development board       | STM32F103C8T6 minimum system board |
| RTOS                    | FreeRTOS                           |
| Display                 | 0.96-inch OLED module              |
| Communication           | UART / RS485                       |
| External storage        | W25Q64 SPI Flash                   |
| PC-side adapter         | USB to RS485 module                |
| Board-side RS485 module | TTL to RS485 module                |

## Peripheral Overview

The current hardware system includes:

* ADC input for analog data acquisition
* OLED display for local status display
* UART1 for RS485 communication
* SPI1 for W25Q64 external Flash
* GPIO for RS485 direction control
* GPIO for LED / key test functions

## Pin Assignment

| Function    | STM32 Pin   | Description                                |
| ----------- | ----------- | ------------------------------------------ |
| USART1_TX   | PA9         | UART1 transmit                             |
| USART1_RX   | PA10        | UART1 receive                              |
| RS485_DE_RE | PB12        | RS485 transmit / receive direction control |
| SPI1_CS     | PA4         | W25Q64 chip select                         |
| SPI1_SCK    | PA5         | W25Q64 SPI clock                           |
| SPI1_MISO   | PA6         | W25Q64 SPI MISO                            |
| SPI1_MOSI   | PA7         | W25Q64 SPI MOSI                            |
| ADC1_IN0    | PA0         | Analog input                               |
| LED         | PA1         | Status LED                                 |
| OLED_SCL    | I2C SCL pin | OLED clock line                            |
| OLED_SDA    | I2C SDA pin | OLED data line                             |

> Note: The actual OLED I2C pins should follow the STM32CubeMX configuration and the current hardware wiring.

## RS485 Connection

The project uses a half-duplex RS485 communication structure.

PC side:

```text
PC
↓
USB to RS485 module
```

STM32 side:

```text
STM32 USART1
↓
TTL to RS485 module
```

RS485 wiring:

| USB-RS485 | TTL-RS485 | Description                 |
| --------- | --------- | --------------------------- |
| A         | A         | RS485 differential signal A |
| B         | B         | RS485 differential signal B |
| GND       | GND       | Common ground, recommended  |

RS485 direction control:

| PB12 Level | RS485 Mode    |
| ---------- | ------------- |
| 0          | Receive mode  |
| 1          | Transmit mode |

In the project, the RS485 driver switches the module to transmit mode before sending data, waits until UART transmission is complete, and then switches back to receive mode.

## W25Q64 Connection

W25Q64 is connected through SPI1.

| W25Q64 Pin | STM32 Pin | Description              |
| ---------- | --------- | ------------------------ |
| CS         | PA4       | Chip select              |
| SCK        | PA5       | SPI clock                |
| MISO       | PA6       | SPI data input to MCU    |
| MOSI       | PA7       | SPI data output from MCU |
| VCC        | 3.3V      | Power supply             |
| GND        | GND       | Ground                   |

W25Q64 is used to store historical log records.

Current usage:

* Store query success logs
* Store protocol-related log data
* Support historical log query by sequence number

## ADC Input

The current baseline version uses `ADC1_IN0` as the analog input channel.

Current test input:

```text
Potentiometer analog voltage
```

Future upgrade direction:

```text
Industrial analog signal
0-3.3V signal conditioning output
0-10V signal converted to 0-3.3V
4-20mA signal converted to voltage by sampling resistor and conditioning circuit
```

## OLED Display

OLED is used for local system display.

Current displayed information:

* ADC value
* Sample counter
* Communication status

Communication status includes:

| Display   | Meaning                                  |
| --------- | ---------------------------------------- |
| IDLE      | No recent valid communication            |
| COMM OK   | A valid communication frame was received |
| PROTO ERR | Protocol error occurred                  |

## Hardware System Structure

```text
PC Upper Computer
↓
USB to RS485 Module
↓
RS485 Bus
↓
TTL to RS485 Module
↓
STM32F103C8T6
├── ADC input
├── OLED display
├── W25Q64 SPI Flash
└── FreeRTOS tasks
```

## Future Hardware Upgrade

Planned hardware upgrade direction:

* Add real RS485 Modbus RTU industrial sensor
* Add RS485 Modbus temperature / humidity transmitter
* Add RS485 electric meter or voltage/current acquisition module
* Add industrial analog input conditioning circuit
* Design custom PCB with RS485 terminal block and W25Q64 circuit
* Add power protection and communication protection circuit
