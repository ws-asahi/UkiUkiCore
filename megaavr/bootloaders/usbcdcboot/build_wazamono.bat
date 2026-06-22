@echo off
setlocal
pushd "%~dp0"
REM ============================================================
REM  build_wazamono.bat   (cmd-native, ASCII-only auto-detect)
REM  WazamonoCore fork - build the USB CDC bootloader for the
REM  Wazamono product boards.
REM
REM  This does NOT modify the clean-room bootloader source. The only
REM  board difference from the generic 64du build is the DFU-status
REM  LED pin, which is passed in at build time:
REM
REM    board            MCU         LED   note
REM    ---------------  ----------  ----  ----------------------------
REM    Wazamono Tachi   avr64du32   PD5   RX LED / LED_BUILTIN, act-LOW
REM    (Wazamono Tsurugi to be added later: avr64du32, LED PD6)
REM
REM  main.c defaults the LED to PA7 and drives it active-LOW; the
REM  Wazamono RX/TX LEDs are also active-LOW (Pro Micro convention),
REM  so only the pin needs to move - no polarity change.
REM
REM  The signature is read from SIGROW at runtime (stk500.c), so the
REM  single avr64du32 hex serves every Wazamono board that shares the
REM  64 KB flash size; the bootloader always reports the true DEVICEID.
REM
REM  Put this in  WazamonoCore\megaavr\bootloaders\usbcdcboot\  and run.
REM ============================================================

REM --- find avr-gcc: 5 levels up (Arduino\tools), version auto-detected
set "GCCBIN="
for /d %%d in ("%~dp0..\..\..\..\..\tools\avr-gcc\*") do set "GCCBIN=%%~fd\bin"
if not defined GCCBIN (
  echo ERROR: avr-gcc toolchain folder not found under
  echo   %~dp0..\..\..\..\..\tools\avr-gcc\
  echo Adjust the path in the for /d line to match your install.
  popd ^& exit /b 1
)
if not exist "%GCCBIN%\avr-gcc.exe" (
  echo ERROR: avr-gcc.exe missing in "%GCCBIN%"
  popd ^& exit /b 1
)
set "PATH=%GCCBIN%;%PATH%"
if not defined MAKE set MAKE=make

REM            class           mcu         LEDport LEDpin
call :build wazamonotachi   avr64du32   PORTD   5

echo.
echo === collecting hex files into ..\hex\ ===
if not exist "..\hex" mkdir "..\hex"
move /y usbcdcboot_*.hex "..\hex\" >nul

echo.
echo === hex files in ..\hex\ ===
dir /b "..\hex\usbcdcboot_wazamono*.hex"
popd
endlocal
goto :eof

:build
REM  %1=class tag   %2=mcu   %3=LED port   %4=LED pin
echo.
echo ------ building %2  -^> usbcdcboot_%1.hex  (LED %3 %4) ------
del /q src\*.o 2>nul
del /q usbcdcboot_%1.elf usbcdcboot_%1.hex usbcdcboot_%1.lst usbcdcboot_%1.map 2>nul
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 LED_PORT=%3 LED_PIN=%4 all
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 size
goto :eof
