@echo off
setlocal

rem Check if an argument is provided
if "%~1"=="" (
    echo Usage: %~nx0 filename
    exit /b
)

set "inputFile=%~1"
set "outputFile=%~n1.exe"

if not exist .\out (
    mkdir out
)

cl.exe /Zi /EHsc /nologo /Fo:.\out\ %inputFile% /link /OUT:.\out\%outputFile%

endlocal
