@echo off
setlocal
pushd "%~dp0"
REM ============================================================
REM  build_ukiukiduino.bat   (cmd-native, ASCII-only auto-detect)
REM  UkiUkiCore - build the USB CDC bootloader for the UkiUkiduino.
REM
REM  This does NOT modify the clean-room bootloader source. Board
REM  parameters are passed in at build time:
REM
REM    board        MCU         LED   pol       VREG  USB ident (VID:PID)
REM    -----------  ----------  ----  --------  ----  -----------------------------
REM    UkiUkiduino  avr64du32   PA0   act-HIGH  1     0x1209:0x000B (test placeholder)
REM
REM    - LED pin     : LED_PORT / LED_PIN
REM    - LED polarity: LED_AH=1 (active-HIGH) | LED_AL=1 (active-LOW)
REM                    Neither given => active-LOW; both given => LED_AH wins.
REM    - USB identity: fixed in src/usb_desc.h (replace the test PID with the
REM      officially assigned pid.codes PID before release)
REM
REM  The UkiUkiduino LED (PA0) is active-HIGH (Arduino Uno "D13" convention).
REM
REM  The signature is read from SIGROW at runtime (stk500.c), so the single
REM
REM  Put this in  UkiUkiCore\megaavr\bootloaders\usbcdcboot\  and run.
REM ============================================================

REM --- find avr-gcc under C:\avr-gcc (version auto-detected) ------------
REM  The avr-gcc 15.2 toolchain lives at  C:\avr-gcc\avr-gcc-15.2.0\ .
REM  Override the root with:  set "AVRGCC_ROOT=D:\path"  before running.
if not defined AVRGCC_ROOT set "AVRGCC_ROOT=C:\avr-gcc"
set "GCCBIN="
for /d %%d in ("%AVRGCC_ROOT%\avr-gcc-*") do set "GCCBIN=%%~fd\bin"
if not defined GCCBIN if exist "%AVRGCC_ROOT%\bin\avr-gcc.exe" set "GCCBIN=%AVRGCC_ROOT%\bin"
if not defined GCCBIN (
  echo ERROR: no avr-gcc-*\bin found under "%AVRGCC_ROOT%"
  echo   Put the toolchain at  C:\avr-gcc\avr-gcc-15.2.0\   ^(or set AVRGCC_ROOT^).
  popd ^& exit /b 1
)
if not exist "%GCCBIN%\avr-gcc.exe" (
  echo ERROR: avr-gcc.exe missing in "%GCCBIN%"
  popd ^& exit /b 1
)
set "PATH=%GCCBIN%;%PATH%"
if not defined MAKE set MAKE=make

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
if "%VREGVAL%"=="" set "VREGVAL=1"
echo.
echo ------ building %2  -^> usbcdcboot_%1.hex  (LED %3 %4, pol %5, VREG %VREGVAL%) ------
del /q src\*.o 2>nul
del /q usbcdcboot_%1.elf usbcdcboot_%1.hex usbcdcboot_%1.lst usbcdcboot_%1.map 2>nul
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 LED_PORT=%3 LED_PIN=%4 VREG=%VREGVAL% %POLFLAG% all
"%MAKE%" MCU=%2 TARGET=usbcdcboot_%1 VREG=%VREGVAL% %POLFLAG% size
goto :eof
