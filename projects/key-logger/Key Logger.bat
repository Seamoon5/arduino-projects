@echo off
title Arduino Key Logger
set "SCRIPT_DIR=C:\Users\Public\key-logger\"
cd /d "%SCRIPT_DIR%"
if errorlevel 1 (
  echo ERROR: Missing folder %SCRIPT_DIR%
  pause
  exit /b 1
)

echo ============================================
echo   Arduino Key Logger
echo   Saves keys you type into the Uno EEPROM
echo   Folder: %CD%
echo ============================================
echo.
echo Hotkeys while running:
echo   F6  read log from Arduino
echo   F7  clear log
echo   F8  pause / resume
echo   F10 quit
echo.
echo Close Arduino Serial Monitor first.
echo.

where python >nul 2>nul
if errorlevel 1 (
  echo ERROR: Python not found on Windows.
  echo Install from https://www.python.org/downloads/
  pause
  exit /b 1
)

python -c "import pynput, serial" >nul 2>nul
if errorlevel 1 (
  echo Installing libraries ^(pynput, pyserial^)...
  python -m pip install -r requirements.txt
  if errorlevel 1 (
    echo ERROR: pip install failed. Run:
    echo   python -m pip install pynput pyserial
    pause
    exit /b 1
  )
)

python key_logger.py %*
if errorlevel 1 (
  echo.
  echo Key logger stopped with an error. See message above.
)
pause
