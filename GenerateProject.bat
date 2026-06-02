@echo off
echo Running Setup Script...
python script/Setup.py
if %errorlevel% neq 0 (
    echo Setup failed!
    pause
    exit /b %errorlevel%
)

echo.
echo Generating Visual Studio 2022 Solution...
cmake -B build -G "Visual Studio 17 2022"
echo.
pause
