@echo off
pushd .
setlocal
if "%1"=="t" goto tgui3
if "%1"=="s" goto nooskewl_shim
if "%1"=="w" goto nooskewl_wedge
if "%1"=="m" goto monster_rpg_3
:tgui3
call b.bat t %2
call i.bat t %2 %3 %4
goto done
:nooskewl_shim
call b.bat s %2
call i.bat s %2 %3 %4
goto done
:nooskewl_wedge
call b.bat w %2
call i.bat w %2 %3 %4
goto done
:monster_rpg_3
call b.bat m %2
call i.bat m %2 %3 %4
goto done
:done
endlocal
popd
