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
REM    board             MCU         LED   pol     USB ident (VID:PID)
REM    ----------------  ----------  ----  ------  -------------------
REM    Wazamono Tachi    avr64du32   PD5   act-LO  0x1209:0x0005
REM    Wazamono Tsurugi  avr64du32   PD6   act-LO  0x1209:0x0007
REM
REM  Per-board build differences are passed in at build time; the
REM  clean-room source is NOT edited per board:
REM    - LED pin    : LED_PORT / LED_PIN
REM    - LED polarity: LED_ACTIVE_HIGH=1   (default = active-LOW)
REM    - USB identity: BOARD=TACHI | TSURUGI  (selects PID + product string)
REM
REM  main.c defaults the LED to PA7 and drives it active-LOW; the Wazamono
REM  RX/TX LEDs are also active-LOW (Pro Micro convention).
REM
REM  NOTE on the Tsurugi LED polarity: this build assumes active-LOW.  The
REM  classic Arduino Uno "D13" LED is active-HIGH - if the Tsurugi schematic
REM  uses that, add "AH" as the 6th argument to its :build call below.
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

REM            class             mcu        LEDport LEDpin board     [LEDpol]
call :build wazamonotachi     avr64du32  PORTD   5      TACHI
call :build wazamonotsurugi   avr64du32  PORTD   6      TSURUGI AH
REM  ^ if the Tsurugi D13 LED is active-HIGH, append "AH" as the 6th arg:
REM      call :build wazamonotsurugi   avr64du32  PORTD   6      TSURUGI   AH

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
REM  %1=class tag  %2=mcu  %3=LED port  %4=LED pin  %5=board tag  %6=LED pol (AH=active-high, optional)
set "POLFLAG="
if /i "%~6"=="AH" set "POLFLAG=LED_ACTIVE_HIGH=1"
echo.
echo ------ building %2  -^> usbcdcboot_%1.hex  (LED %3 %4, board %5 %6) ------
del /q src\*.o 2>nul
del /q usbcdcboot_%1.elf usbcdcboot_%1.hex usbcdcboot_%1.lst usbcdcboot_%1.map 2>nul
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 LED_PORT=%3 LED_PIN=%4 BOARD=%5 %POLFLAG% all
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 BOARD=%5 %POLFLAG% size
goto :eof
