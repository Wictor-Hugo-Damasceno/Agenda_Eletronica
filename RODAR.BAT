@echo off

set PATH=%CD%\dependencias\compiler\bin;%PATH%


echo Compilando o projeto...
make


if %ERRORLEVEL% EQU 0 (
    echo.
    echo =========================================
    echo   Compilado com sucesso! Abrindo agenda...
    echo =========================================
    
    agenda.exe
) else (
    echo.
    echo #########################################
    echo   ERRO NA COMPILACAO! Verifique o codigo.
    echo #########################################
    pause
)