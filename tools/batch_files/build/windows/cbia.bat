@echo off
pushd .
setlocal
call ca.bat %1 %4
call ba.bat %1
call ia.bat %1 %2 %3
endlocal
popd
