@echo off
REM This batch file diagnoses brick.jpg loading issues
REM Run this from the Graphics-Project directory

setlocal enabledelayedexpansion

echo.
echo ================================
echo  BRICK.JPG DIAGNOSTIC TOOL
echo ================================
echo.

REM Check if we're in the right directory
if not exist "textures\brick.jpg" (
    echo [ERROR] textures\brick.jpg not found!
    echo.
    echo Current directory: %cd%
    echo.
    echo Please run this script from: C:\Users\USER\Desktop\Graphics-Project
    pause
    exit /b 1
)

echo [OK] Found textures directory
echo [OK] Found brick.jpg

REM Get file properties
for %%A in ("textures\brick.jpg") do (
    echo.
    echo File Properties:
    echo   - Full path: %%~fA
    echo   - Size: %%~zA bytes
    echo   - Modified: %%~tA
    echo.
)

REM Check other critical texture files
echo Checking other required textures:
setlocal
for %%F in (
    "textures\floor_tiles_texture.jpg"
    "textures\task_stamping.jpg"
    "textures\task_welding.jpg"
    "textures\task_inspection.jpg"
    "textures\task_packing.jpg"
    "textures\task_shipping.jpg"
    "textures\map.jpg"
) do (
    if exist "%%F" (
        echo   [?] %%F found
    ) else (
        echo   [?] %%F MISSING
    )
)

echo.
echo ================================
echo  DIAGNOSTIC COMPLETE
echo ================================
echo.
echo Next steps:
echo   1. Open Visual Studio project
echo   2. Go to Project Properties
echo   3. Set Debugging Working Directory to:
echo      C:\Users\USER\Desktop\Graphics-Project
echo   4. Rebuild and run
echo.
pause
