@echo off
pushd .
setlocal
call b.bat t %1
call b.bat s %1
call b.bat w %1
call b.bat m %1
endlocal
popd
