@echo off
title CPU Warning Light
set "SCRIPT_DIR=%~dp0"

:: WSL UNC paths (\\wsl.localhost\...) cannot be used as CMD folders.
:: Fall back to the Windows copy of this project.
if "%SCRIPT_DIR:~0,2%"=="\\" (
  echo Detected WSL path — switching to Windows copy...
  set "SCRIPT_DIR=C:\Users\Public\cpu-warning\"
)

cd /d "%SCRIPT_DIR%" 2>nul
if errorlevel 1 (
  echo ERROR: Cannot open folder:
  echo   %SCRIPT_DIR%
  echo Use this file instead:
  echo   C:\Users\Public\cpu-warning\CPU Warning.bat
  pause
  exit /b 1
)

echo ============================================
echo   CPU Warning Light
echo   LED ON when CPU above 60%%
echo   Folder: %CD%
echo ============================================
echo.

where python >nul 2>nul
if errorlevel 1 (
  echo ERROR: Python not found on Windows.
  echo Install from https://www.python.org/downloads/
  pause
  exit /b 1
)

python -c "import psutil, serial" >nul 2>nul
if errorlevel 1 (
  echo Installing libraries ^(psutil, pyserial^)...
  python -m pip install -r requirements.txt
  if errorlevel 1 (
    echo ERROR: pip install failed. Run:
    echo   python -m pip install psutil pyserial
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
