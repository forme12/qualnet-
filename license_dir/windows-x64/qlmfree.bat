@echo off

if not exist qlmlist.bat goto curdir
if "%3" == "" goto help

echo Attempting to free active license ...
echo.

lmutil lmremove -c .. qualnet %1 %2 %3


:curdir

echo qlmlist.bat must be in current directory
goto end

:help

echo To free an active license, run qlmlist.bat to list active licenses.  Then run:
echo.
echo     qlmfree user host display
echo.
echo The user, host, and display parameters are listed in order for you
echo in the qlmlist output.

goto end

:end
