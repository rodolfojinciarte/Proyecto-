@echo off
REM =====================================================================
REM QUE HACE: ejecuta los 8 casos de uso con los archivos entrada_*.txt
REM y guarda lo que muestra el programa en salida_*.txt.
REM Cada caso empieza con la carpeta datos/ vacia (se borran los .bin).
REM Al terminar, datos/ queda con los datos de ejemplo del caso 8.
REM Se ejecuta desde la carpeta Taller:  pruebas\ejecutar_pruebas.bat
REM =====================================================================
cd /d "%~dp0.."
if not exist taller.exe (
    echo ERROR: primero compile el programa para crear taller.exe
    exit /b 1
)
if not exist datos mkdir datos

REM ---------- Caso de uso 1 ----------
echo Caso de uso 1...
del /q datos\*.bin 2>nul
taller.exe < pruebas\entrada_1a.txt > pruebas\salida_1a.txt
taller.exe < pruebas\entrada_1b.txt > pruebas\salida_1b.txt
REM ---------- Caso de uso 2 ----------
echo Caso de uso 2...
del /q datos\*.bin 2>nul
taller.exe < pruebas\entrada_2a.txt > pruebas\salida_2a.txt
REM ---------- Caso de uso 3 ----------
echo Caso de uso 3...
del /q datos\*.bin 2>nul
taller.exe < pruebas\entrada_3a.txt > pruebas\salida_3a.txt
taller.exe < pruebas\entrada_3b.txt > pruebas\salida_3b.txt
taller.exe < pruebas\entrada_3c.txt > pruebas\salida_3c.txt
REM ---------- Caso de uso 4 ----------
echo Caso de uso 4...
del /q datos\*.bin 2>nul
taller.exe < pruebas\entrada_4a.txt > pruebas\salida_4a.txt
taller.exe < pruebas\entrada_4b.txt > pruebas\salida_4b.txt
REM ---------- Caso de uso 5 ----------
echo Caso de uso 5...
del /q datos\*.bin 2>nul
taller.exe < pruebas\entrada_5a.txt > pruebas\salida_5a.txt
taller.exe < pruebas\entrada_5b.txt > pruebas\salida_5b.txt
taller.exe < pruebas\entrada_5c.txt > pruebas\salida_5c.txt
REM ---------- Caso de uso 6 ----------
echo Caso de uso 6...
del /q datos\*.bin 2>nul
taller.exe < pruebas\entrada_6a.txt > pruebas\salida_6a.txt
taller.exe < pruebas\entrada_6b.txt > pruebas\salida_6b.txt
REM ---------- Caso de uso 7 ----------
echo Caso de uso 7...
del /q datos\*.bin 2>nul
taller.exe < pruebas\entrada_7a.txt > pruebas\salida_7a.txt
REM Caso 7: se reemplaza vehiculos.bin por la copia del backup mas reciente
for /f "delims=" %%d in ('dir /b /ad /o:n backup_*') do set ULTIMO=%%d
copy /y "%ULTIMO%\vehiculos.bin" datos\vehiculos.bin >nul
taller.exe < pruebas\entrada_7b.txt > pruebas\salida_7b.txt
REM ---------- Caso de uso 8 ----------
echo Caso de uso 8...
del /q datos\*.bin 2>nul
taller.exe < pruebas\entrada_8a.txt > pruebas\salida_8a.txt
taller.exe < pruebas\entrada_8b.txt > pruebas\salida_8b.txt

echo Listo. Las salidas quedaron en la carpeta pruebas y los datos de ejemplo en datos.
