@echo off
setlocal
cd /d "%~dp0"
echo ===================================================
echo   Compilando Nintendo DS Recomp - Golden Sun 3...
echo ===================================================

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
    )
)

if not exist build (
    mkdir build
)

cmake -B build -S . -A x64
if %errorlevel% neq 0 (
    echo Error al configurar el proyecto con CMake.
    pause
    exit /b %errorlevel%
)

cmake --build build --config Release
if %errorlevel% neq 0 (
    echo Error al compilar el ejecutable.
    pause
    exit /b %errorlevel%
)

if exist "build\Release\GoldenSun3_Recomp.exe" (
    copy /y "build\Release\GoldenSun3_Recomp.exe" "GoldenSun3_Recomp.exe" >nul
)

echo.
echo ===================================================
echo   Compilacion finalizada con exito!
echo   Ejecutable generado: GoldenSun3_Recomp.exe
echo ===================================================
pause
