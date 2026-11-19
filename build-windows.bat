@echo off
REM Compila BassForge.vst3 en Windows.
REM Requiere: Visual Studio 2022 (o Build Tools) con "Desarrollo para el escritorio con C++", CMake y Git.
cd /d "%~dp0"
cmake -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto error
cmake --build build --config Release --target BassForge_VST3
if errorlevel 1 goto error
echo.
echo ===============================================================
echo  Listo. Tu plugin esta en:
echo  build\BassForge_artefacts\Release\VST3\BassForge.vst3
echo  Copialo a C:\Program Files\Common Files\VST3\
echo ===============================================================
pause
exit /b 0
:error
echo.
echo Hubo un error al compilar. Revisa el mensaje de arriba.
pause
exit /b 1
