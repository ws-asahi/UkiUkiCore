@echo off
setlocal EnableExtensions
REM ============================================================================
REM  setup_toolchain.bat  -  UkiUkiCore 手動インストール用ツールチェーン配置スクリプト(Windows)
REM
REM  wazamono-toolchain のリリースから avr-gcc / avrdude をダウンロードし、
REM  このリポジトリ直下の tools\ に Arduino IDE が認識する形で配置します:
REM
REM      <スケッチブック>\hardware\UkiUkiCore\
REM          megaavr\                        <- コア(platform.txt など)
REM          tools\avr-gcc\15.2.0-wazamono2\ <- bin\avr-gcc.exe ...
REM          tools\avrdude\8.1-wazamono2\    <- bin\avrdude.exe, etc\avrdude.conf
REM
REM  Arduino IDE 2 / arduino-cli は hardware\<VENDOR>\tools\<name>\<version>\ を
REM  {runtime.tools.<name>-<version>.path} として登録するため、コンパイル・
REM  スケッチ書き込み・ブートローダ書き込みの全てでこのツールが使われます。
REM  (platform.local.txt は不要です。あれば削除します。)
REM
REM  使い方:  setup_toolchain.bat [--force]
REM      --force   既に配置済みでも削除して入れ直す
REM
REM  必要なもの: Windows 10 1803 以降(標準の curl.exe / tar.exe / certutil を使用)
REM  日本語を含むパス(ドキュメント\Arduino など)でも動作します。
REM ============================================================================

REM ---- バージョン(docs\package_ukiuki_index.json と揃えること) --------------
set "TAG=tools-15.2.0-wazamono2"
set "GCC_VER=15.2.0-wazamono2"
set "DUDE_VER=8.1-wazamono2"
set "HOST=x86_64-mingw32"
set "BASE_URL=https://github.com/ws-asahi/wazamono-toolchain/releases/download/%TAG%"

set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
set "DEST=%ROOT%\tools"
set "FORCE=0"
if /i "%~1"=="--force" set "FORCE=1"
if /i "%~1"=="-h"     goto :usage
if /i "%~1"=="--help" goto :usage

where curl.exe >nul 2>&1 || (echo ERROR: curl.exe not found. Windows 10 1803 or later is required.& exit /b 1)
where tar.exe  >nul 2>&1 || (echo ERROR: tar.exe not found. Windows 10 1803 or later is required.& exit /b 1)

set "WORK=%TEMP%\ukiuki_toolchain_%RANDOM%"
mkdir "%WORK%" || exit /b 1

echo Fetching checksums.txt ...
curl.exe -fL -s -o "%WORK%\checksums.txt" "%BASE_URL%/checksums.txt" || (echo ERROR: download failed: checksums.txt& goto :fail)

call :install avrdude "%DUDE_VER%" "avrdude-%DUDE_VER%-%HOST%.tar.gz" "bin\avrdude.exe" || goto :fail
call :install avr-gcc "%GCC_VER%"  "avr-gcc-%GCC_VER%-%HOST%.tar.gz"  "bin\avr-gcc.exe"  || goto :fail

REM ---- 旧方式の platform.local.txt を片付ける ----------------------------
if exist "%ROOT%\megaavr\platform.local.txt" (
  del /q "%ROOT%\megaavr\platform.local.txt"
  echo [clean] removed obsolete megaavr\platform.local.txt ^(no longer needed^)
)

echo.
echo Installed under: %DEST%
dir /b "%DEST%"
echo.
echo Restart the Arduino IDE. The build/upload log should show tools under
echo   ...\hardware\UkiUkiCore\tools\avr-gcc\%GCC_VER%\  and  ...\tools\avrdude\%DUDE_VER%\
rd /s /q "%WORK%" >nul 2>&1
endlocal
exit /b 0

REM ----------------------------------------------------------------------------
REM :install <name> <version> <archive> <probe-relative-path>
REM ----------------------------------------------------------------------------
:install
set "T_NAME=%~1"
set "T_VER=%~2"
set "T_ARCHIVE=%~3"
set "T_PROBE=%~4"
set "T_TARGET=%DEST%\%T_NAME%\%T_VER%"
if exist "%T_TARGET%\%T_PROBE%" if "%FORCE%"=="0" (
  echo [skip] %T_NAME% %T_VER% is already installed at %T_TARGET%
  exit /b 0
)
echo [get ] %T_ARCHIVE%
curl.exe -fL -# -o "%WORK%\%T_ARCHIVE%" "%BASE_URL%/%T_ARCHIVE%" || (echo ERROR: download failed: %T_ARCHIVE%& exit /b 1)

REM --- SHA-256 検証 (certutil の 2 行目がハッシュ) ---
set "WANT="
for /f "tokens=1,2" %%a in ('findstr /c:" %T_ARCHIVE%" "%WORK%\checksums.txt"') do if /i "%%b"=="%T_ARCHIVE%" set "WANT=%%a"
set "HAVE="
for /f "skip=1 tokens=1" %%h in ('certutil -hashfile "%WORK%\%T_ARCHIVE%" SHA256 ^| findstr /v /i "certutil"') do if not defined HAVE set "HAVE=%%h"
if defined WANT if defined HAVE (
  if /i not "%WANT%"=="%HAVE%" (
    echo ERROR: SHA-256 mismatch for %T_ARCHIVE%
    echo   expected %WANT%
    echo   got      %HAVE%
    exit /b 1
  )
  echo [ok  ] SHA-256 verified
) else (
  echo [warn] could not verify SHA-256; continuing
)

echo [untar] %T_ARCHIVE%
REM 配置先と同じドライブ上に展開してから改名する(TEMP が別ドライブでも move で失敗しないように)
set "T_STAGE=%DEST%\%T_NAME%\.staging"
if exist "%T_STAGE%" rd /s /q "%T_STAGE%"
mkdir "%T_STAGE%" || (echo ERROR: cannot create %T_STAGE%& exit /b 1)
tar.exe -xzf "%WORK%\%T_ARCHIVE%" -C "%T_STAGE%" || (echo ERROR: extract failed& exit /b 1)
REM アーカイブ直下は <name>-<version>\ の 1 ディレクトリ
set "T_TOP="
for /d %%d in ("%T_STAGE%\*") do if not defined T_TOP set "T_TOP=%%~fd"
if not defined T_TOP (echo ERROR: unexpected archive layout& exit /b 1)
if not exist "%T_TOP%\%T_PROBE%" (echo ERROR: unexpected archive layout ^(no %T_PROBE%^)& exit /b 1)
if exist "%T_TARGET%" rd /s /q "%T_TARGET%"
move /y "%T_TOP%" "%T_TARGET%" >nul || (echo ERROR: move failed& exit /b 1)
rd /s /q "%T_STAGE%" >nul 2>&1
echo [done] %T_NAME% %T_VER% -^> %T_TARGET%
exit /b 0

:usage
for /f "tokens=* delims=" %%l in ('findstr /b /c:"REM " "%~f0"') do echo %%l
exit /b 0

:fail
echo.
echo Setup FAILED. Nothing may have been changed; re-run after fixing the error above.
rd /s /q "%WORK%" >nul 2>&1
endlocal
exit /b 1
