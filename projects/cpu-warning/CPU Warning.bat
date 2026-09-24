@echo off
title CPU Warning Light
cd /d "%~dp0"

where python >nul 2>nul
if errorlevel 1 (
  echo Python not found. Install Python 3 from python.org first.
  pause
  exit /b 1
)

python -c "import psutil, serial" >nul 2>nul
if errorlevel 1 (
  echo Installing required libraries (psutil, pyserial)...
  python -m pip install -r requirements.txt
)

python cpu_sender.py %*
pause
