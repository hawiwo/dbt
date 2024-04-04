@echo off & setlocal enableextensions
set bcc_drive=%CD:~0,2%
call "C:\Program Files (x86)\Embarcadero\Studio\23.0\bin\rsvars.bat"

if "%1"=="test" (
    set _version=test
)

msbuild.exe C:\data\dbt\dbt.cbproj /t:build /p:Config=Release /p:platform=Win64

rem Überprüfe den errorlevel
if %errorlevel% neq 0 (
    echo Build fehlgeschlagen mit Fehlerlevel %errorlevel%.
) else (
    echo ===================================================================================
    echo Build erfolgreich.
    echo ===================================================================================
    copy /y C:\data\dbt\Win64\Release\dbt.exe \\ul-dc-07\C$\usr\
)




