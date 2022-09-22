@echo off
pushd .
setlocal
call ba.bat %1
call iag.bat %1 %2
endlocal
popd
