"""STM32F103 industrial gateway PC upper monitor."""

from __future__ import annotations

import queue
import threading
import tkinter as tk
from pathlib import Path
from tkinter import messagebox, ttk
from typing import Any

try:
    import serial
    from serial.tools import list_ports
except ImportError:  # Allows protocol helpers to be tested before pyserial is installed.
    serial = None
    list_ports = None


HEAD1 = 0xAA
HEAD2 = 0x55

CMD_GET_SENSOR = 0x01
CMD_RESP_SENSOR = 0x81
CMD_GET_LOG = 0x02
CMD_RESP_LOG = 0x82
CMD_RESP_ERROR = 0xE0

GET_SENSOR_FRAME = bytes.fromhex("AA 55 01 01 C1 E0")
ERROR_CODE_NAMES = {
    0x01: "帧头错误",
    0x02: "长度错误",
    0x03: "命令错误",
    0x04: "CRC 错误",
}


class ProtocolError(ValueError):
    """Raised when a protocol frame is malformed."""


def crc16_modbus(data: bytes) -> int:
    """Calculate CRC16-Modbus (initial 0xFFFF, polynomial 0xA001)."""
    crc = 0xFFFF
    for value in data:
        crc ^= value
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc


def build_get_sensor_frame() -> bytes:
    """Build the fixed current-sensor query frame."""
    return GET_SENSOR_FRAME


def build_get_log_frame(seq: int) -> bytes:
    """Build a history-log query frame for an unsigned 32-bit sequence."""
    if not isinstance(seq, int) or isinstance(seq, bool):
        raise ValueError("日志序号必须是整数")
    if not 0 <= seq <= 0xFFFFFFFF:
        raise ValueError("日志序号范围必须为 0 到 4294967295")
    body = bytes((0x05, CMD_GET_LOG)) + seq.to_bytes(4, "big")
    crc = crc16_modbus(body)
    return bytes((HEAD1, HEAD2)) + body + crc.to_bytes(2, "little")


def _validate_frame(frame: bytes, expected_len: int, expected_cmd: int) -> None:
    if len(frame) < 5:
        raise ProtocolError("响应帧过短")
    if frame[:2] != bytes((HEAD1, HEAD2)):
        raise ProtocolError("帧头错误，应为 AA 55")
    actual_total = frame[2] + 5
    if len(frame) != actual_total:
        raise ProtocolError(
            f"长度错误：LEN={frame[2]} 时整帧应为 {actual_total} 字节，实际 {len(frame)} 字节"
        )
    if frame[2] != expected_len:
        raise ProtocolError(f"LEN 错误：期望 0x{expected_len:02X}，收到 0x{frame[2]:02X}")
    if frame[3] != expected_cmd:
        raise ProtocolError(f"命令错误：期望 0x{expected_cmd:02X}，收到 0x{frame[3]:02X}")
    if not verify_crc(frame):
        raise ProtocolError("CRC ERROR")


def verify_crc(frame: bytes) -> bool:
    """Verify frame CRC and structural length."""
    if len(frame) < 5 or frame[:2] != bytes((HEAD1, HEAD2)):
        return False
    if len(frame) != frame[2] + 5:
        return False
    expected = crc16_modbus(frame[2:-2])
    received = int.from_bytes(frame[-2:], "little")
    return expected == received


def parse_sensor_response(frame: bytes) -> dict[str, int]:
    """Validate and parse a 10-byte sensor response."""
    _validate_frame(frame, expected_len=0x05, expected_cmd=CMD_RESP_SENSOR)
    return {
        "cnt": int.from_bytes(frame[4:6], "big"),
        "adc": int.from_bytes(frame[6:8], "big"),
    }


def parse_log_response(frame: bytes) -> dict[str, int]:
    """Validate and parse a 16-byte history-log response."""
    _validate_frame(frame, expected_len=0x0B, expected_cmd=CMD_RESP_LOG)
    return {
        "seq": int.from_bytes(frame[4:8], "big"),
        "type": frame[8],
        "cnt": int.from_bytes(frame[9:11], "big"),
        "adc": int.from_bytes(frame[11:13], "big"),
        "err": frame[13],
    }


def parse_error_response(frame: bytes) -> int:
    """Validate an error response and return its error code."""
    _validate_frame(frame, expected_len=0x02, expected_cmd=CMD_RESP_ERROR)
    return frame[4]


def format_hex(data: bytes) -> str:
    return " ".join(f"{value:02X}" for value in data)


class UpperMonitorApp:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("工业数据采集网关上位机")
        icon_path = Path(__file__).resolve().parent / "assets" / "industrial_gateway_icon.ico"
        if icon_path.exists():
            try:
                self.root.iconbitmap(default=str(icon_path))
            except tk.TclError:
                pass
        self.root.geometry("850x650")
        self.root.minsize(760, 570)

        self.serial_port: Any = None
        self.busy = False
        self.ui_events: queue.Queue[tuple[str, Any]] = queue.Queue()

        self.port_var = tk.StringVar()
        self.baud_var = tk.StringVar(value="115200")
        self.connection_var = tk.StringVar(value="未连接")
        self.cnt_var = tk.StringVar(value="--")
        self.adc_var = tk.StringVar(value="--")
        self.seq_var = tk.StringVar(value="0")

        self._build_ui()
        self.refresh_ports()
        self.root.after(50, self._process_ui_events)
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

    def _build_ui(self) -> None:
        container = ttk.Frame(self.root, padding=10)
        container.pack(fill="both", expand=True)

        connection = ttk.LabelFrame(container, text="串口连接", padding=8)
        connection.pack(fill="x")
        ttk.Label(connection, text="串口：").grid(row=0, column=0, padx=4, pady=4)
        self.port_combo = ttk.Combobox(
            connection, textvariable=self.port_var, width=18, state="readonly"
        )
        self.port_combo.grid(row=0, column=1, padx=4, pady=4)
        ttk.Button(connection, text="刷新", command=self.refresh_ports).grid(
            row=0, column=2, padx=4
        )
        ttk.Label(connection, text="波特率：").grid(row=0, column=3, padx=(18, 4))
        self.baud_combo = ttk.Combobox(
            connection,
            textvariable=self.baud_var,
            values=("9600", "19200", "38400", "57600", "115200"),
            width=10,
        )
        self.baud_combo.grid(row=0, column=4, padx=4)
        self.connect_button = ttk.Button(connection, text="连接", command=self.toggle_connection)
        self.connect_button.grid(row=0, column=5, padx=(18, 4))
        ttk.Label(connection, textvariable=self.connection_var).grid(row=0, column=6, padx=8)

        sensor = ttk.LabelFrame(container, text="当前数据", padding=8)
        sensor.pack(fill="x", pady=(10, 0))
        ttk.Label(sensor, text="采样计数 CNT：").grid(row=0, column=0, padx=4, pady=4)
        ttk.Label(sensor, textvariable=self.cnt_var, width=12).grid(row=0, column=1)
        ttk.Label(sensor, text="ADC：").grid(row=0, column=2, padx=(30, 4))
        ttk.Label(sensor, textvariable=self.adc_var, width=12).grid(row=0, column=3)
        self.sensor_button = ttk.Button(
            sensor, text="查询当前数据", command=self.query_current_sensor
        )
        self.sensor_button.grid(row=0, column=4, padx=(30, 4))

        logs = ttk.LabelFrame(container, text="历史日志", padding=8)
        logs.pack(fill="both", expand=True, pady=(10, 0))
        ttk.Label(logs, text="日志序号 seq：").pack(side="top", anchor="w")
        query_line = ttk.Frame(logs)
        query_line.pack(fill="x", pady=(3, 8))
        ttk.Entry(query_line, textvariable=self.seq_var, width=20).pack(side="left")
        self.log_button = ttk.Button(query_line, text="查询日志", command=self.query_log)
        self.log_button.pack(side="left", padx=8)

        columns = ("seq", "type", "cnt", "adc", "err")
        self.log_table = ttk.Treeview(logs, columns=columns, show="headings", height=7)
        headings = {"seq": "SEQ", "type": "TYPE", "cnt": "CNT", "adc": "ADC", "err": "ERR"}
        for column in columns:
            self.log_table.heading(column, text=headings[column])
            self.log_table.column(column, width=110, anchor="center")
        self.log_table.pack(fill="both", expand=True)

        raw = ttk.LabelFrame(container, text="原始收发帧", padding=8)
        raw.pack(fill="both", expand=True, pady=(10, 0))
        self.raw_text = tk.Text(raw, height=10, wrap="word", state="disabled")
        scrollbar = ttk.Scrollbar(raw, orient="vertical", command=self.raw_text.yview)
        self.raw_text.configure(yscrollcommand=scrollbar.set)
        self.raw_text.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

    def refresh_ports(self) -> None:
        ports = [] if list_ports is None else [item.device for item in list_ports.comports()]
        self.port_combo["values"] = ports
        if ports and self.port_var.get() not in ports:
            self.port_var.set(ports[0])
        elif not ports:
            self.port_var.set("")

    def toggle_connection(self) -> None:
        if self.serial_port is not None and self.serial_port.is_open:
            self.disconnect()
        else:
            self.connect()

    def connect(self) -> None:
        if serial is None:
            messagebox.showerror("缺少依赖", "未安装 pyserial，请先执行 pip install -r requirements.txt")
            return
        port = self.port_var.get().strip()
        if not port:
            messagebox.showwarning("串口未选择", "未发现或未选择可用串口")
            return
        try:
            baudrate = int(self.baud_var.get())
            self.serial_port = serial.Serial(
                port=port,
                baudrate=baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.5,
                write_timeout=0.5,
            )
        except (ValueError, serial.SerialException) as exc:
            self.serial_port = None
            messagebox.showerror("连接失败", str(exc))
            return
        self.connect_button.configure(text="断开")
        self.connection_var.set(f"已连接 {port}")
        self._append_raw(f"已连接：{port}，{baudrate} 8N1")

    def disconnect(self) -> None:
        if self.serial_port is not None:
            try:
                self.serial_port.close()
            except serial.SerialException:
                pass
        self.serial_port = None
        self.connect_button.configure(text="连接")
        self.connection_var.set("未连接")
        self._append_raw("串口已断开")

    def query_current_sensor(self) -> None:
        self._start_request(build_get_sensor_frame())

    def query_log(self) -> None:
        try:
            text = self.seq_var.get().strip()
            if not text:
                raise ValueError("请输入日志序号")
            seq = int(text, 0)
            frame = build_get_log_frame(seq)
        except ValueError as exc:
            messagebox.showwarning("输入错误", str(exc))
            return
        self._start_request(frame)

    def _start_request(self, frame: bytes) -> None:
        if self.serial_port is None or not self.serial_port.is_open:
            messagebox.showwarning("串口未连接", "请先连接串口")
            return
        if self.busy:
            messagebox.showinfo("正在通信", "请等待当前查询完成")
            return
        self.busy = True
        self._set_query_buttons(False)
        threading.Thread(target=self._request_worker, args=(frame,), daemon=True).start()

    def _request_worker(self, frame: bytes) -> None:
        try:
            port = self.serial_port
            port.reset_input_buffer()
            port.write(frame)
            port.flush()
            self.ui_events.put(("raw", f"TX: {format_hex(frame)}"))
            response = self._read_frame(port)
            self.ui_events.put(("raw", f"RX: {format_hex(response)}"))
            if not verify_crc(response):
                self.ui_events.put(("raw", "CRC ERROR"))
                raise ProtocolError("响应帧 CRC 校验失败")
            self.ui_events.put(("raw", "CRC OK"))

            command = response[3]
            if command == CMD_RESP_SENSOR:
                self.ui_events.put(("sensor", parse_sensor_response(response)))
            elif command == CMD_RESP_LOG:
                self.ui_events.put(("log", parse_log_response(response)))
            elif command == CMD_RESP_ERROR:
                error_code = parse_error_response(response)
                detail = ERROR_CODE_NAMES.get(error_code, "未知错误")
                raise ProtocolError(f"下位机返回错误码 0x{error_code:02X}（{detail}）")
            else:
                raise ProtocolError(f"未知响应命令 0x{command:02X}")
        except TimeoutError as exc:
            self.ui_events.put(("error", ("接收超时", str(exc))))
        except ProtocolError as exc:
            self.ui_events.put(("error", ("协议错误", str(exc))))
        except Exception as exc:
            self.ui_events.put(("error", ("串口通信错误", str(exc))))
        finally:
            self.ui_events.put(("done", None))

    @staticmethod
    def _read_frame(port: Any) -> bytes:
        """Find AA 55, then read one complete LEN-delimited frame."""
        matched_head1 = False
        while True:
            chunk = port.read(1)
            if not chunk:
                raise TimeoutError("未在超时时间内收到响应")
            value = chunk[0]
            if not matched_head1:
                matched_head1 = value == HEAD1
            elif value == HEAD2:
                break
            else:
                matched_head1 = value == HEAD1

        length_raw = port.read(1)
        if not length_raw:
            raise TimeoutError("收到帧头，但未收到 LEN 字段")
        length = length_raw[0]
        if length < 1:
            raise ProtocolError("LEN 字段不能小于 1")
        remaining = bytearray()
        expected = length + 2  # CMD+DATA plus two CRC bytes.
        while len(remaining) < expected:
            chunk = port.read(expected - len(remaining))
            if not chunk:
                partial = bytes((HEAD1, HEAD2, length)) + bytes(remaining)
                raise TimeoutError(f"响应不完整：{format_hex(partial)}")
            remaining.extend(chunk)
        return bytes((HEAD1, HEAD2, length)) + bytes(remaining)

    def _process_ui_events(self) -> None:
        try:
            while True:
                event, payload = self.ui_events.get_nowait()
                if event == "raw":
                    self._append_raw(payload)
                elif event == "sensor":
                    self.cnt_var.set(str(payload["cnt"]))
                    self.adc_var.set(str(payload["adc"]))
                elif event == "log":
                    self.log_table.insert(
                        "",
                        0,
                        values=(
                            payload["seq"],
                            f"0x{payload['type']:02X}",
                            payload["cnt"],
                            payload["adc"],
                            f"0x{payload['err']:02X}",
                        ),
                    )
                elif event == "error":
                    title, detail = payload
                    self._append_raw(f"ERROR: {detail}")
                    messagebox.showerror(title, detail)
                elif event == "done":
                    self.busy = False
                    self._set_query_buttons(True)
        except queue.Empty:
            pass
        self.root.after(50, self._process_ui_events)

    def _set_query_buttons(self, enabled: bool) -> None:
        state = "normal" if enabled else "disabled"
        self.sensor_button.configure(state=state)
        self.log_button.configure(state=state)

    def _append_raw(self, text: str) -> None:
        self.raw_text.configure(state="normal")
        self.raw_text.insert("end", text + "\n")
        self.raw_text.see("end")
        self.raw_text.configure(state="disabled")

    def _on_close(self) -> None:
        if self.serial_port is not None:
            try:
                self.serial_port.close()
            except Exception:
                pass
        self.root.destroy()


def main() -> None:
    root = tk.Tk()
    UpperMonitorApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
