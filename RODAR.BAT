@echo off

set PATH=%CD%\compiler\bin;%PATH%


echo Compilando o projeto...
make

:: 3. Verifica se a compilacao deu certo
if %ERRORLEVEL% EQU 0 (
    echo.
    echo =========================================
    echo   Compilado com sucesso! Abrindo agenda...
    echo =========================================
    :: Executa o seu programa
    agenda.exe
) else (
    echo.
    echo #########################################
    echo   ERRO NA COMPILACAO! Verifique o codigo.
    echo #########################################
    pause
)