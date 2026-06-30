@echo off
echo Building Taiv Account Tracker with MinGW...
echo.

g++ -std=c++17 -municode -DUNICODE -D_UNICODE AccountTracker.cpp -o TaivAccountTracker.exe -mwindows -lcomctl32 -lshell32 -lole32 -lgdi32 -luser32

if exist TaivAccountTracker.exe (
    echo.
    echo Build successful: TaivAccountTracker.exe
    echo Run create_desktop_shortcut.ps1 to create a desktop shortcut.
) else (
    echo.
    echo Build failed. Make sure g++ is installed and available in PATH.
)

pause
