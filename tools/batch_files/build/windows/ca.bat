@echo off
pushd .
setlocal
call c.bat t %1 %2
call c.bat s %1 %2
call c.bat w %1 %2
call c.bat m %1 %2
call c.bat d %1 %2
endlocal
popd
