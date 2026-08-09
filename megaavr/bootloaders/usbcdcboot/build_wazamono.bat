@echo off
setlocal
pushd "%~dp0"
REM ============================================================
REM  build_wazamono.bat   (cmd-native, ASCII-only source)
REM  WazamonoCore fork - build the USB CDC bootloader for the
REM  Wazamono product boards.
REM
REM  This does NOT modify the clean-room bootloader source. The only
REM  per-board build differences are passed in at build time:
REM
REM    board             MCU         LED   pol       VREG  USB ident (VID:PID)
REM    ----------------  ----------  ----  --------  ----  -------------------
REM    Wazamono Tachi    avr64du32   PF3   act-LOW   0     0x1209:0x0005
REM    Wazamono Tsurugi  avr64du32   PD6   act-HIGH  0     0x1209:0x0007
REM    Wazamono Kunai    avr32du20   PD4   act-LOW   0     0x1209:0x0009
REM
REM    - LED pin     : LED_PORT / LED_PIN
REM    - LED polarity: LED_AH=1 (active-HIGH) | LED_AL=1 (active-LOW)
REM                    Neither given => active-LOW; both given => LED_AH wins.
REM    - USB identity: BOARD=TACHI | TSURUGI  (selects PID + product string)
REM
REM  Tachi's LED (PF3 = D17, LED_BUILTIN = RX LED, rev.4) is active-LOW;
REM  Tsurugi's LED (PD6 = D13, op-amp buffered) is active-HIGH (Arduino Uno
REM  convention). Each board passes its polarity explicitly below. All boards
REM  Tachi/Kunai feed VUSB from an external 3.3 V LDO (VREG=0); the Tsurugi
REM  uses the internal USB regulator at VDD = 5 V (VREG=1, power config 5b).
REM
REM  The signature is read from SIGROW at runtime (stk500.c). Tachi (rev.4)
REM  and Tsurugi both target avr64du32 but keep separate hexes (different
REM  LED pin/polarity and USB identity).
REM
REM  --- Toolchain search order (first hit wins) -----------------------
REM    0) %AVRGCC_ROOT%                     explicit override
REM    1) Board Manager install (canonical since the package release):
REM       %LOCALAPPDATA%\Arduino15\packages\WazamonoCore\tools\avr-gcc\*
REM    2) Sketchbook tools folder, resolved RELATIVE to this script so the
REM       ASCII-only source still finds a Japanese profile path:
REM       ..\..\..\..\..\tools\avr-gcc\*-wazamono*
REM       (usbcdcboot -> bootloaders -> megaavr -> WazamonoCore -> hardware
REM        -> Arduino\tools)
REM    3) Legacy stock build at C:\avr-gcc\avr-gcc-*
REM  Wazamono builds (*-wazamonoN) run from any path (uniform ANSI codepage
REM  + binutils >= 2.46.1); a stock build should stay on an ASCII path.
REM
REM  Put this in  WazamonoCore\megaavr\bootloaders\usbcdcboot\  and run.
REM ============================================================

if not defined AVRGCC_ROOT set "AVRGCC_ROOT=C:\avr-gcc"
set "GCCBIN="

REM 0) explicit override: accept <root>\bin, <root>\avr-gcc-*, <root>\*-wazamono*
if defined AVRGCC_ROOT (
  for /d %%d in ("%AVRGCC_ROOT%\*-wazamono*") do set "GCCBIN=%%~fd\bin"
  if not defined GCCBIN for /d %%d in ("%AVRGCC_ROOT%\avr-gcc-*") do set "GCCBIN=%%~fd\bin"
  if not defined GCCBIN if exist "%AVRGCC_ROOT%\bin\avr-gcc.exe" set "GCCBIN=%AVRGCC_ROOT%\bin"
)
REM 1) Board Manager install
if not defined GCCBIN for /d %%d in ("%LOCALAPPDATA%\Arduino15\packages\WazamonoCore\tools\avr-gcc\*") do set "GCCBIN=%%~fd\bin"
REM 2) sketchbook tools, relative to this script
if not defined GCCBIN for /d %%d in ("%~dp0..\..\..\..\..\tools\avr-gcc\*-wazamono*") do set "GCCBIN=%%~fd\bin"

if not defined GCCBIN (
  echo ERROR: no avr-gcc toolchain found. Looked in:
  echo   "%AVRGCC_ROOT%"  ^(override root^)
  echo   "%LOCALAPPDATA%\Arduino15\packages\WazamonoCore\tools\avr-gcc\*\bin"
  echo   "%~dp0..\..\..\..\..\tools\avr-gcc\*-wazamono*\bin"
  echo   "C:\avr-gcc\avr-gcc-*\bin"
  echo Install WazamonoCore via the Board Manager, or set AVRGCC_ROOT.
  popd ^& exit /b 1
)
if not exist "%GCCBIN%\avr-gcc.exe" (
  echo ERROR: avr-gcc.exe missing in "%GCCBIN%"
  popd ^& exit /b 1
)
echo Using avr-gcc: %GCCBIN%\avr-gcc.exe
set "PATH=%GCCBIN%;%PATH%"
if not defined MAKE set MAKE=make

REM            class             mcu        LEDport LEDpin board     LEDpol(AH|AL) VREG(0|1)
call :build wazamonotachi     avr64du32  PORTF   3      TACHI   AL 0
call :build wazamonotsurugi   avr64du32  PORTD   6      TSURUGI AH 1
call :build wazamonokunai     avr32du20  PORTD   4      KUNAI   AL 0

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
REM  %1=class tag  %2=mcu  %3=LED port  %4=LED pin  %5=board tag  %6=LED pol (AH | AL)  %7=VREG (0 | 1)
set "POLFLAG="
if /i "%~6"=="AH" set "POLFLAG=LED_AH=1"
if /i "%~6"=="AL" set "POLFLAG=LED_AL=1"
set "VREGVAL=%~7"
if not defined VREGVAL set "VREGVAL=0"
echo.
echo ------ building %2  -^> usbcdcboot_%1.hex  (LED %3 %4, board %5 %6, VREG %VREGVAL%) ------
del /q src\*.o 2>nul
del /q usbcdcboot_%1.elf usbcdcboot_%1.hex usbcdcboot_%1.lst usbcdcboot_%1.map 2>nul
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 LED_PORT=%3 LED_PIN=%4 BOARD=%5 VREG=%VREGVAL% %POLFLAG% all
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 BOARD=%5 VREG=%VREGVAL% %POLFLAG% size
goto :eof
