@echo off
setlocal
rem Kit aprovado em Downloads; instala somente DADOS\LOGO\PEGASO_V1 da candidata 19.8.
rem Requer o repositorio atualizado na branch experimental.
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0SCRIPTS\INSTALAR_PEGASO_DOWNLOADS_19_8.ps1" %*
set "EXIT_CODE=%ERRORLEVEL%"
if not "%EXIT_CODE%"=="0" echo ERRO: identidade nao instalada. Veja a mensagem acima.
exit /b %EXIT_CODE%
