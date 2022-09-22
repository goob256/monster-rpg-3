@echo off
pushd .
setlocal
call c.bat %1 %2
call b.bat %1 %2
call i.bat %1 %2 %3
endlocal
popd
