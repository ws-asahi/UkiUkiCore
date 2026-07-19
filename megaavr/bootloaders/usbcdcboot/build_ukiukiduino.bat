@echo off
setlocal
pushd "%~dp0"
REM ============================================================
REM  build_ukiukiduino.bat   (cmd-native, ASCII-only source)
REM  UkiUkiCore - build the USB CDC bootloader for the UkiUkiduino.
REM
REM  This does NOT modify the clean-room bootloader source. Board
REM  parameters are passed in at build time:
REM
REM    board        MCU         LED             VREG  USB ident (VID:PID)
REM    -----------  ----------  --------------  ----  -----------------------------
REM    UkiUkiduino  avr64du32   PA0 (WS2812D)   1     0x1209:0x000B (test placeholder)
REM
REM    The on-board LED is a WS2812D-F5 addressable RGB LED (LED_WS2812=1):
REM    DFU mode breathes yellow; the AH polarity flag is ignored in this mode.
REM
REM    - LED pin     : LED_PORT / LED_PIN
REM    - LED polarity: LED_AH=1 (active-HIGH) | LED_AL=1 (active-LOW)
REM                    Neither given => active-LOW; both given => LED_AH wins.
REM    - VREG        : 1 = internal VUSB regulator (the UkiUkiduino)
REM    - USB identity: fixed in src/usb_desc.h (replace the test PID with the
REM      officially assigned pid.codes PID before release)
REM
REM  The UkiUkiduino LED (PA0) is active-HIGH (Arduino Uno "D13" convention).
REM
REM  --- Toolchain search order (first hit wins) -----------------------
REM    0) %AVRGCC_ROOT%                     explicit override
REM    1) Board Manager install (UkiUkiCore first, then WazamonoCore -
REM       both packages ship the same wazamono toolchain):
REM       %LOCALAPPDATA%\Arduino15\packages\UkiUkiCore\tools\avr-gcc\*
REM       %LOCALAPPDATA%\Arduino15\packages\WazamonoCore\tools\avr-gcc\*
REM    2) Sketchbook tools folder, resolved RELATIVE to this script so the
REM       ASCII-only source still finds a Japanese profile path:
REM       ..\..\..\..\..\tools\avr-gcc\*-wazamono*
REM       (usbcdcboot -> bootloaders -> megaavr -> UkiUkiCore -> hardware
REM        -> Arduino\tools)
REM    3) Legacy stock build at C:\avr-gcc\avr-gcc-*
REM  Wazamono builds (*-wazamonoN) run from any path (uniform ANSI codepage
REM  + binutils >= 2.46.1); a stock build should stay on an ASCII path.
REM
REM  Put this in  UkiUkiCore\megaavr\bootloaders\usbcdcboot\  and run.
REM ============================================================

if not defined AVRGCC_ROOT set "AVRGCC_ROOT=C:\avr-gcc"
set "GCCBIN="

REM 0) explicit override: accept <root>\bin, <root>\avr-gcc-*, <root>\*-wazamono*
if defined AVRGCC_ROOT (
  for /d %%d in ("%AVRGCC_ROOT%\*-wazamono*") do set "GCCBIN=%%~fd\bin"
  if not defined GCCBIN for /d %%d in ("%AVRGCC_ROOT%\avr-gcc-*") do set "GCCBIN=%%~fd\bin"
  if not defined GCCBIN if exist "%AVRGCC_ROOT%\bin\avr-gcc.exe" set "GCCBIN=%AVRGCC_ROOT%\bin"
)
REM 1) Board Manager install (UkiUkiCore, then WazamonoCore)
if not defined GCCBIN for /d %%d in ("%LOCALAPPDATA%\Arduino15\packages\UkiUkiCore\tools\avr-gcc\*") do set "GCCBIN=%%~fd\bin"
if not defined GCCBIN for /d %%d in ("%LOCALAPPDATA%\Arduino15\packages\WazamonoCore\tools\avr-gcc\*") do set "GCCBIN=%%~fd\bin"
REM 2) sketchbook tools, relative to this script
if not defined GCCBIN for /d %%d in ("%~dp0..\..\..\..\..\tools\avr-gcc\*-wazamono*") do set "GCCBIN=%%~fd\bin"

if not defined GCCBIN (
  echo ERROR: no avr-gcc toolchain found. Looked in:
  echo   "%AVRGCC_ROOT%"  ^(override root^)
  echo   "%LOCALAPPDATA%\Arduino15\packages\UkiUkiCore\tools\avr-gcc\*\bin"
  echo   "%LOCALAPPDATA%\Arduino15\packages\WazamonoCore\tools\avr-gcc\*\bin"
  echo   "%~dp0..\..\..\..\..\tools\avr-gcc\*-wazamono*\bin"
  echo   "C:\avr-gcc\avr-gcc-*\bin"
  echo Install UkiUkiCore/WazamonoCore via the Board Manager, or set AVRGCC_ROOT.
  popd ^& exit /b 1
)
if not exist "%GCCBIN%\avr-gcc.exe" (
  echo ERROR: avr-gcc.exe missing in "%GCCBIN%"
  popd ^& exit /b 1
)
echo Using avr-gcc: %GCCBIN%\avr-gcc.exe
set "PATH=%GCCBIN%;%PATH%"
if not defined MAKE set MAKE=make

REM            class             mcu        LEDport LEDpin LEDpol(AH|AL) VREG(0|1)
call :build ukiukiduino       avr64du32  PORTA   0      AH      1

echo.
echo === collecting hex files into ..\hex\ ===
if not exist "..\hex" mkdir "..\hex"
move /y usbcdcboot_*.hex "..\hex\" >nul

echo.
echo === hex files in ..\hex\ ===
dir /b "..\hex\usbcdcboot_ukiukiduino.hex"
popd
endlocal
goto :eof

:build
REM  %1=class tag  %2=mcu  %3=LED port  %4=LED pin  %5=LED pol (AH | AL)  %6=VREG (0 | 1)
set "POLFLAG="
if /i "%~5"=="AH" set "POLFLAG=LED_AH=1"
if /i "%~5"=="AL" set "POLFLAG=LED_AL=1"
set "VREGVAL=%~6"
if not defined VREGVAL set "VREGVAL=1"
echo.
echo ------ building %2  -^> usbcdcboot_%1.hex  (LED %3 %4, pol %5, VREG %VREGVAL%) ------
del /q src\*.o 2>nul
del /q usbcdcboot_%1.elf usbcdcboot_%1.hex usbcdcboot_%1.lst usbcdcboot_%1.map 2>nul
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 LED_PORT=%3 LED_PIN=%4 VREG=%VREGVAL% LED_WS2812=1 %POLFLAG% all
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 VREG=%VREGVAL% LED_WS2812=1 %POLFLAG% size
goto :eof
