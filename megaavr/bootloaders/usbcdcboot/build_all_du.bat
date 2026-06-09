
@echo off
setlocal
pushd "%~dp0"
REM ============================================================
REM  build_all_du.bat   (cmd-native, ASCII-only auto-detect)
REM  Builds the USB CDC bootloader for every AVR DU device as
REM  usbcdcboot_<chip>.hex  (optiboot-style naming) and collects
REM  them into  ..\hex\  (alongside optiboot_*.hex).
REM  Put this in  DxCore\megaavr\bootloaders\usbcdcboot\  and run it.
REM
REM  Chip list below uses the bare tag (no "avr"); each build sets
REM  MCU=avr<tag> and TARGET=usbcdcboot_<tag>, so the hex is named to
REM  match boards.txt:  avrduusb.bootloader.class=usbcdcboot_<tag>.
REM  Cleans with 'del' and runs 'make all' only (avoids rm/awk); the
REM  link step still prints section sizes via avr-size -A.
REM ============================================================

REM --- find avr-gcc: 5 levels up (Arduino\tools), version auto-detected
set "GCCBIN="
for /d %%d in ("%~dp0..\..\..\..\..\tools\avr-gcc\*") do set "GCCBIN=%%~fd\bin"
if not defined GCCBIN (
  echo ERROR: avr-gcc toolchain folder not found under
  echo   %~dp0..\..\..\..\..\tools\avr-gcc\
  echo Adjust the path in the for /d line to match your install.
  popd & exit /b 1
)
if not exist "%GCCBIN%\avr-gcc.exe" (
  echo ERROR: avr-gcc.exe missing in "%GCCBIN%"
  popd & exit /b 1
)
set "PATH=%GCCBIN%;%PATH%"
if not defined MAKE set MAKE=make.exe

call :build 16du14
call :build 16du20
call :build 16du28
call :build 16du32
call :build 32du14
call :build 32du20
call :build 32du28
call :build 32du32
call :build 64du28
call :build 64du32

echo.
echo === collecting hex files into ..\hex\ ===
if not exist "..\hex" mkdir "..\hex"
move /y usbcdcboot_*.hex "..\hex\" >nul

echo.
echo === hex files in ..\hex\ ===
dir /b "..\hex\usbcdcboot_*.hex"
popd
endlocal
goto :eof

:build
echo.
echo ------ building avr%1  to  usbcdcboot_%1.hex ------
del /q src\*.o 2>nul
del /q usbcdcboot_%1.elf usbcdcboot_%1.hex usbcdcboot_%1.lst usbcdcboot_%1.map 2>nul
"%MAKE%" MCU=avr%1 TARGET=usbcdcboot_%1 all
goto :eof