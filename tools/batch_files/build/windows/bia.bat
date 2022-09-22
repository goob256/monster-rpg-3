@echo off
pushd .
setlocal
call ba.bat %1
call ia.bat %1 %2 %3
endlocal
popd
