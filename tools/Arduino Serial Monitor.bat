@echo off
title Arduino Uno Serial Monitor
echo Finding your Arduino Uno...
C:\Users\Public\arduino-cli.exe board list --discovery-timeout 5s > "%TEMP%\arduino-detect.txt" 2>&1
for /f "tokens=1" %%P in ('findstr /r "^COM[0-9]" "%TEMP%\arduino-detect.txt"') do set PORT=%%P
del "%TEMP%\arduino-detect.txt" >nul 2>&1
if "%PORT%"=="" (
    echo.
    echo No Arduino board found! Make sure it is plugged in, then try again.
    echo.
    pause
    exit /b
)
echo.
echo Board found on %PORT%. Opening serial monitor at 9600 baud...
echo.
echo Type a command and press Enter:
echo   1 = LED on       0 = LED off       b = blink       f = fade
echo Close this window to exit.
echo.
C:\Users\Public\arduino-cli.exe monitor -p %PORT% -b arduino:avr:uno --config baudrate=9600