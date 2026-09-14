@echo off
setlocal EnableExtensions
REM ============================================================================
REM  tools\setup_toolchain.bat  -  toolchain installer for a MANUAL (sketchbook) install
REM
REM  Reads docs\package_ukiuki_index.json (the Boards Manager index), takes the
REM  avr-gcc / avrdude versions required by the newest platform (toolsDependencies)
REM  and places them in THIS directory (hardware\UkiUkiCore\tools\) in the layout
REM  the Arduino IDE recognises:
REM
REM      <sketchbook>\hardware\UkiUkiCore\
REM          megaavr\                        <- the core (platform.txt ...)
REM          tools\avr-gcc\<version>\        <- bin\avr-gcc.exe ...
REM          tools\avrdude\<version>\        <- bin\avrdude.exe, etc\avrdude.conf
REM
REM  Arduino IDE 2 / arduino-cli registers hardware\<VENDOR>\tools\<name>\<version>\
REM  as {runtime.tools.<name>-<version>.path}, so compile, sketch upload AND
REM  burn-bootloader all use these tools. platform.local.txt is no longer
REM  needed (it is deleted if present).
REM
REM  Usage:  tools\setup_toolchain.bat [--force]
REM      --force   reinstall even if the tool is already present
REM
REM  Requires Windows 10 1803 or later (built-in curl.exe / tar.exe / certutil /
REM  PowerShell). Works from paths containing non-ASCII characters.
REM
REM  NOTE: keep this file ASCII-only. cmd.exe reads batch files in the console
REM  code page (CP932 on Japanese Windows); UTF-8 comments get mis-parsed and
REM  fragments of them are executed as commands.
REM ============================================================================

set "DEST=%~dp0"
set "DEST=%DEST:~0,-1%"
for %%i in ("%DEST%\..") do set "ROOT=%%~fi"
set "INDEX=%ROOT%\docs\package_ukiuki_index.json"
set "HOST=x86_64-mingw32"
set "FORCE=0"
if /i "%~1"=="--force" set "FORCE=1"
if /i "%~1"=="-h"     goto :usage
if /i "%~1"=="--help" goto :usage

if not exist "%INDEX%" (echo ERROR: index not found: %INDEX%& exit /b 1)
where curl.exe >nul 2>&1 || (echo ERROR: curl.exe not found. Windows 10 1803 or later is required.& exit /b 1)
where tar.exe  >nul 2>&1 || (echo ERROR: tar.exe not found. Windows 10 1803 or later is required.& exit /b 1)
where powershell.exe >nul 2>&1 || (echo ERROR: powershell.exe not found.& exit /b 1)

set "WORK=%TEMP%\ukiuki_toolchain_%RANDOM%"
mkdir "%WORK%" || exit /b 1

echo Index : %INDEX%
echo Host  : %HOST%

REM ---- Parse the index with PowerShell: one line per tool (name|version|archive|url|sha256) ----
REM      Uses toolsDependencies of the newest platform. url is empty when no archive exists for HOST.
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop';" ^
  "$d = Get-Content -Raw -Encoding UTF8 '%INDEX%' | ConvertFrom-Json;" ^
  "function VKey($v){ ($v -replace '-','.') -split '\.' | ForEach-Object { if ($_ -match '^\d+$') { '{0:D6}' -f [int]$_ } else { $_ } } }" ^
  "$pls = @(); foreach ($p in $d.packages) { $pls += $p.platforms };" ^
  "$best = $pls | Sort-Object { (VKey $_.version) -join '.' } | Select-Object -Last 1;" ^
  "$tools = @(); foreach ($p in $d.packages) { $tools += $p.tools };" ^
  "$out = @();" ^
  "foreach ($dep in $best.toolsDependencies) {" ^
  "  $t = $tools | Where-Object { $_.name -eq $dep.name -and $_.version -eq $dep.version } | Select-Object -First 1;" ^
  "  $s = $null; if ($t) { $s = $t.systems | Where-Object { $_.host -eq '%HOST%' } | Select-Object -First 1 };" ^
  "  if ($s) { $out += ($dep.name,$dep.version,$s.archiveFileName,$s.url,($s.checksum -replace '^SHA-256:','')) -join '|' }" ^
  "  else    { $out += ($dep.name,$dep.version,'','','') -join '|' }" ^
  "};" ^
  "Set-Content -Path '%WORK%\tools.lst' -Value $out -Encoding ASCII;" ^
  "Write-Host ('Platform: ' + $best.version)" ^
  || (echo ERROR: failed to parse %INDEX%& goto :fail)

set "INSTALLED="
for /f "usebackq tokens=1-5 delims=|" %%a in ("%WORK%\tools.lst") do (
  call :install "%%a" "%%b" "%%c" "%%d" "%%e" || goto :fail
)

REM ---- Check that platform.txt pins the same versions ------------------------
if exist "%ROOT%\megaavr\platform.txt" for %%t in (%INSTALLED%) do (
  findstr /c:"runtime.tools.%%t.path" "%ROOT%\megaavr\platform.txt" >nul || echo [warn] megaavr\platform.txt does not reference {runtime.tools.%%t.path} - update the pin to match the index.
)

REM ---- Remove the obsolete platform.local.txt --------------------------------
if exist "%ROOT%\megaavr\platform.local.txt" (
  del /q "%ROOT%\megaavr\platform.local.txt"
  echo [clean] removed obsolete megaavr\platform.local.txt ^(no longer needed^)
)

echo.
echo Tools under %DEST%:
for /d %%t in ("%DEST%\*") do for /d %%v in ("%%t\*") do echo   %%~nxt\%%~nxv
echo.
echo Restart the Arduino IDE. The build/upload log should show tools under ...\hardware\UkiUkiCore\tools\
rd /s /q "%WORK%" >nul 2>&1
endlocal
exit /b 0

REM ----------------------------------------------------------------------------
REM :install <name> <version> <archive> <url> <sha256>
REM ----------------------------------------------------------------------------
:install
set "T_NAME=%~1"
set "T_VER=%~2"
set "T_ARCHIVE=%~3"
set "T_URL=%~4"
set "WANT=%~5"
set "T_TARGET=%DEST%\%T_NAME%\%T_VER%"
set "T_PROBE=bin\%T_NAME%.exe"
if "%T_URL%"=="" (
  echo [note] %T_NAME% %T_VER%: no archive for host %HOST% in the index - skipped.
  exit /b 0
)
if exist "%T_TARGET%\%T_PROBE%" if "%FORCE%"=="0" (
  echo [skip] %T_NAME% %T_VER% is already installed at %T_TARGET%
  set "INSTALLED=%INSTALLED% %T_NAME%-%T_VER%"
  exit /b 0
)
echo [get ] %T_ARCHIVE%
curl.exe -fL -# -o "%WORK%\%T_ARCHIVE%" "%T_URL%" || (echo ERROR: download failed: %T_ARCHIVE%& exit /b 1)

REM --- SHA-256 check (take the hash line from certutil output) ---
set "HAVE="
for /f "skip=1 tokens=1" %%h in ('certutil -hashfile "%WORK%\%T_ARCHIVE%" SHA256 ^| findstr /v /i "certutil"') do if not defined HAVE set "HAVE=%%h"
if not "%WANT%"=="" if not "%HAVE%"=="" (
  if /i not "%WANT%"=="%HAVE%" (
    echo ERROR: SHA-256 mismatch for %T_ARCHIVE%
    echo   expected %WANT%
    echo   got      %HAVE%
    exit /b 1
  )
  echo [ok  ] SHA-256 verified
)
if "%HAVE%"=="" echo [warn] could not verify SHA-256; continuing

echo [untar] %T_ARCHIVE%
REM Extract on the destination drive, then rename (move fails across drives if TEMP is elsewhere)
set "T_STAGE=%DEST%\%T_NAME%\.staging"
if exist "%T_STAGE%" rd /s /q "%T_STAGE%"
mkdir "%T_STAGE%" || (echo ERROR: cannot create %T_STAGE%& exit /b 1)
tar.exe -xzf "%WORK%\%T_ARCHIVE%" -C "%T_STAGE%" || (echo ERROR: extract failed& exit /b 1)
REM The archive contains a single top-level directory <name>-<version>\
set "T_TOP="
for /d %%d in ("%T_STAGE%\*") do if not defined T_TOP set "T_TOP=%%~fd"
if not defined T_TOP (echo ERROR: unexpected archive layout& exit /b 1)
if not exist "%T_TOP%\%T_PROBE%" (echo ERROR: unexpected archive layout ^(no %T_PROBE%^)& exit /b 1)
if exist "%T_TARGET%" rd /s /q "%T_TARGET%"
move /y "%T_TOP%" "%T_TARGET%" >nul || (echo ERROR: move failed& exit /b 1)
rd /s /q "%T_STAGE%" >nul 2>&1
set "INSTALLED=%INSTALLED% %T_NAME%-%T_VER%"
echo [done] %T_NAME% %T_VER% -^> %T_TARGET%
exit /b 0

:usage
for /f "tokens=* delims=" %%l in ('findstr /b /c:"REM " "%~f0"') do echo %%l
exit /b 0

:fail
echo.
echo Setup FAILED. Re-run after fixing the error above.
rd /s /q "%WORK%" >nul 2>&1
endlocal
exit /b 1
