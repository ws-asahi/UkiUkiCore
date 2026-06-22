@echo off
setlocal
pushd "%~dp0"
REM ============================================================
REM  make_platform_local.bat
REM  Place in  WazamonoCore\megaavr\  and run once.
REM
REM  WHY: WazamonoCore is a manual (sketchbook hardware) install, so the
REM  IDE does NOT resolve the azduino toolchain dependency. platform.txt's
REM  {runtime.tools.avr-gcc.path} then falls back to the stock Arduino
REM  avr-gcc 7.3.0, which has no AVR DU support -> the build fails with
REM    "device-specs/specs-avr64du32: No such file or directory".
REM
REM  This script auto-detects the azduino toolchain you already copied to
REM  Arduino\tools\ (the same one build_wazamono.bat uses) and writes a
REM  platform.local.txt that overrides compiler.path (and avrdude, if
REM  present) to point at it. platform.local.txt is environment-specific
REM  (absolute paths) and should be listed in .gitignore.
REM ============================================================

REM  megaavr -> WazamonoCore -> hardware -> Arduino -> tools
set "TOOLS=%~dp0..\..\..\tools"

REM --- avr-gcc (required for compiling) ---
set "GCCDIR="
for /d %%d in ("%TOOLS%\avr-gcc\*") do set "GCCDIR=%%~fd"
if not defined GCCDIR goto :nogcc
set "GCCFWD=%GCCDIR:\=/%"

REM --- avrdude (optional; needed to UPLOAD over the USB-CDC bootloader) ---
set "DUDEDIR="
for /d %%d in ("%TOOLS%\avrdude\*") do set "DUDEDIR=%%~fd"

> platform.local.txt echo # WazamonoCore platform.local.txt  (auto-generated - add to .gitignore)
>> platform.local.txt echo # Forces the build to use the azduino toolchain (has avr64du32 device-specs)
>> platform.local.txt echo # instead of the IDE default avr-gcc 7.3.0, which lacks AVR DU support.
>> platform.local.txt echo compiler.path=%GCCFWD%/bin/
if not defined DUDEDIR goto :nodude
set "DUDEFWD=%DUDEDIR:\=/%"
>> platform.local.txt echo # avrdude with avr64du32 support (sketch upload + burn bootloader)
>> platform.local.txt echo tools.avrdude.path=%DUDEFWD%
goto :done

:nodude
>> platform.local.txt echo # NOTE: no avrdude found under Arduino\tools\avrdude\ . Compiling will work,
>> platform.local.txt echo # but UPLOADING a sketch over the bootloader needs an avrdude that knows
>> platform.local.txt echo # avr64du32 (>= 7.x). Copy the azduino avrdude there and re-run this script,
>> platform.local.txt echo # then uncomment a line like:  tools.avrdude.path=C:/.../Arduino/tools/avrdude/<ver>

:done
echo.
echo Wrote platform.local.txt in "%CD%":
echo --------------------------------------------------
type platform.local.txt
echo --------------------------------------------------
echo Restart the Arduino IDE (or re-open the sketch) so it re-reads platform.local.txt.
popd & endlocal & goto :eof

:nogcc
echo ERROR: no avr-gcc folder found under "%TOOLS%\avr-gcc\"
echo Copy the azduino8 toolchain there (the same one build_wazamono.bat used
echo successfully to build the bootloader), then re-run this script.
popd & endlocal & exit /b 1
