@echo off
cd /d "%~dp0"
if exist "GoldenSun3_Recomp.exe" (
    start "" "GoldenSun3_Recomp.exe"
    exit
) else if exist "build\Release\GoldenSun3_Recomp.exe" (
    start "" "build\Release\GoldenSun3_Recomp.exe"
    exit
) else (
    echo El ejecutable no ha sido compilado aun.
    echo Ejecuta primero COMPILAR.bat
    pause
)
