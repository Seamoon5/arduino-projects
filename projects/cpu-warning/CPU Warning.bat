@echo off
title CPU Warning Light
cd /d "%~dp0"

echo ============================================
echo   CPU Warning Light
echo   LED turns ON when CPU is above 60%%
echo ============================================
echo.

where python >nul 2>nul
if errorlevel 1 (
  echo ERROR: Python not found on Windows.
  echo Install Python 3 from https://www.python.org/downloads/
  echo During install, tick "Add python.exe to PATH".
  pause
  exit /b 1
)

python -c "import psutil, serial" >nul 2>nul
if errorlevel 1 (
  echo Installing required libraries ^(psutil, pyserial^)...
  python -m pip install -r requirements.txt
  if errorlevel 1 (
    echo ERROR: Could not install libraries.
    echo Try manually:  python -m pip install psutil pyserial
    pause
    exit /b 1
  )
)

echo Starting CPU sender... keep this window open.
echo Close Serial Monitor first if it is open.
echo.
python cpu_sender.py %*
if errorlevel 1 (
  echo.
  echo CPU sender stopped with an error. See message above.
)
pause
