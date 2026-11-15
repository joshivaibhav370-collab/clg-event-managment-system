@echo off
echo Building CampusConnect...
g++ -std=c++17 main.cpp CampusConnect.cpp -o CampusConnect.exe -lws2_32
if errorlevel 1 (
    echo BUILD FAILED - check the errors above.
    pause
    exit /b
)
echo Build successful! Now double-click run.bat
pause
