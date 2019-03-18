@echo off

if not exist qlmlist.bat goto help

echo Listing status of QualNet licenses ... (this may take 10-20 seconds)
echo.

lmutil lmstat -c .. -f

goto end

:help
echo qlmlist.bat must be in current directory
goto end

:end
