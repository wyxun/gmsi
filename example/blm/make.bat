@echo off
setlocal

:: --- Configuration ---
:: Default SW_ROOT if not already set in environment
if "%SW_ROOT%"=="" set SW_ROOT=D:\software

set MAKE_EXE="%SW_ROOT%\msys64\mingw64\bin\mingw32-make.exe"

:: --- Path Validation ---
if not exist %MAKE_EXE% (
    echo.
    echo [ERROR] mingw32-make.exe not found at:
    echo         %MAKE_EXE%
    echo.
    echo [TIP]   Please set the SW_ROOT environment variable correctly, 
    echo         or edit the path in this make.bat file.
    echo         Current SW_ROOT: %SW_ROOT%
    echo.
    pause
    exit /b 1
)

:: --- Argument Handling ---
if "%1"=="auto" goto auto_flow
if "%1"=="rttv" goto gui_flow

:: Default: Pass all arguments to mingw32-make
%MAKE_EXE% %*
goto :eof

:: --- Workflows ---

:auto_flow
echo [INFO] Starting Full Auto Build, Flash and Debug sequence...
%MAKE_EXE% clean
%MAKE_EXE%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %ERRORLEVEL%
)

%MAKE_EXE% flash
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Flashing failed.
    pause
    exit /b %ERRORLEVEL%
)

echo [INFO] Launching RTT Server in background...
start "MODUS RTT Server" cmd /c %MAKE_EXE% rtt

echo [INFO] Waiting for Server to initialize...
timeout /t 3 /nobreak > nul

echo [INFO] Launching RTT Viewer...
start powershell -NoProfile -ExecutionPolicy Bypass -File ".\.agent\workflows\rtt_viewer.ps1"
goto :eof

:gui_flow
echo [INFO] Launching RTT Viewer...
if not exist ".\.agent\workflows\rtt_viewer.ps1" (
    echo [ERROR] rtt_viewer.ps1 not found in .agent/workflows/
    pause
    exit /b 1
)
start powershell -NoProfile -ExecutionPolicy Bypass -File ".\.agent\workflows\rtt_viewer.ps1"
goto :eof
