@echo off
pushd .
setlocal
call i.bat t %1 %2 %3
call i.bat s %1 %2 %3
call i.bat w %1 %2 %3
call i.bat m %1 %2 %3
call i.bat d %1 %2 %3
endlocal
popd
