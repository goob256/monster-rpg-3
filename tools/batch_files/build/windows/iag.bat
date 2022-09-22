@echo off
pushd .
setlocal
call i.bat t %1 %2
call i.bat s %1 %2
call i.bat w %1 %2
call i.bat m %1 %2
endlocal
popd
