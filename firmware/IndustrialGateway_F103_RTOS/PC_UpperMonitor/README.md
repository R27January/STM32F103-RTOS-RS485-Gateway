# 工业数据采集网关上位机

基于 Python、Tkinter 和 pyserial 的最小可用上位机。它按 STM32 下位机现有协议发送 HEX 帧，支持当前数据与 W25Q64 历史日志查询，不修改或扩展下位机协议。

## 环境

- Python 3.9 或更高版本
- Windows（使用 COM 串口）
- Python 自带 Tkinter
- USB-RS485 转换器及对应驱动

## 安装与运行

### 当前电脑直接启动

项目目录中已经创建本地 Python 环境并安装 pyserial。双击 `start_upper_monitor.bat` 即可启动，也可以在 PowerShell 中执行：

```powershell
.\start_upper_monitor.bat
```

### 其他电脑首次安装

在项目根目录打开 PowerShell：

```powershell
cd PC_UpperMonitor
python -m pip install -r requirements.txt
python upper_monitor.py
```

串口配置为 115200、8 数据位、无校验、1 停止位（8N1），接收超时为 0.5 秒。

## 测试查询当前数据

1. 将 PC 的 USB-RS485 转换器正确连接到 STM32 的 RS485 A/B 端，并给下位机上电。
2. 启动程序，在串口下拉框选择转换器对应的 COM 口，波特率保持 `115200`，点击“连接”。
3. 点击“查询当前数据”。
4. 原始帧区应显示：

   ```text
   TX: AA 55 01 01 C1 E0
   RX: AA 55 05 81 ... ... ... ... CRC_L CRC_H
   CRC OK
   ```

5. 界面的 CNT 和 ADC 字段会更新。无响应、帧长错误或 CRC 错误时会弹出提示，并在原始帧区记录错误。

## 测试查询 seq=1 的历史日志

1. 保持串口已连接。
2. 在“日志序号 seq”输入 `1`，点击“查询日志”。
3. 程序自动计算 CRC，原始帧区应显示：

   ```text
   TX: AA 55 05 02 00 00 00 01 B8 4E
   ```

4. 收到有效的 `0x82` 响应后，表格新增一行，显示 SEQ、TYPE、CNT、ADC 和 ERR。
5. 如果收到 `0xE0` 错误响应，程序会校验该帧 CRC，并显示错误码及已知含义。

日志序号支持十进制输入，也支持带 `0x` 前缀的十六进制输入，允许范围为 `0` 到 `4294967295`。

## 协议实现

`upper_monitor.py` 中包含以下独立协议函数，便于直接导入测试：

- `crc16_modbus(data)`
- `build_get_sensor_frame()`
- `build_get_log_frame(seq)`
- `verify_crc(frame)`
- `parse_sensor_response(frame)`
- `parse_log_response(frame)`

CRC16-Modbus 初值为 `0xFFFF`、多项式为 `0xA001`，计算范围为 `LEN + CMD + DATA`，线上顺序为低字节在前。

## 常见问题

- 下拉框没有串口：确认 USB-RS485 驱动已安装，然后点击“刷新”。
- 无响应：检查 COM 口、115200 8N1、A/B 接线、共地和 STM32 是否已运行。
- CRC ERROR：确认下位机与上位机使用同一协议版本，且串口链路没有截断或混入其他数据。
- 串口被占用：关闭串口助手或其他已打开该 COM 口的程序后重试。
