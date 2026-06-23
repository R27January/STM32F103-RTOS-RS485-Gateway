@echo off
cd /d "%~dp0"

if not exist ".venv\Scripts\python.exe" (
    echo Local Python environment not found: .venv
    echo Please create the environment and install requirements first.
    pause
    exit /b 1
)

".venv\Scripts\python.exe" upper_monitor.py

if errorlevel 1 pause
