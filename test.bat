@echo off

.\build_win\out\got.exe -r hey %*

if %errorlevel%==3 (
    for /f %%i in ('.\build_win\out\got.exe') do cd /D "%%i"
)
