@echo off
echo Building Taiv Account Tracker with Microsoft Visual C++...
echo.

cl /EHsc /std:c++17 /DUNICODE /D_UNICODE AccountTracker.cpp /link comctl32.lib shell32.lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /OUT:TaivAccountTracker.exe

if exist TaivAccountTracker.exe (
    echo.
    echo Build successful: TaivAccountTracker.exe
    echo Run create_desktop_shortcut.ps1 to create a desktop shortcut.
) else (
    echo.
    echo Build failed. Make sure you are running this from Visual Studio Developer Command Prompt.
)

pause
