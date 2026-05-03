@echo off
:: Set MSYS2 path (Update this if your installation is elsewhere)
set MSYS64_PATH=D:\software\msys64

:: Add MinGW-w64 and MSYS2 tools to PATH for this session
set PATH=%MSYS64_PATH%\mingw64\bin;%MSYS64_PATH%\usr\bin;%PATH%

echo [INFO] Building SuperWaveform...
make %*

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Build complete.
pause
