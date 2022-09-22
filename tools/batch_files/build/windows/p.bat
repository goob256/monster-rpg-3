@echo off
pushd .
setlocal
if "%1"=="t" goto tgui3
if "%1"=="s" goto nooskewl_shim
if "%1"=="w" goto nooskewl_wedge
if "%1"=="m" goto monster_rpg_3
:tgui3
cd c:\users\trent\code\tgui3
git pull --rebase
goto done
:nooskewl_shim
cd c:\users\trent\code\nooskewl_shim
git pull --rebase
goto done
:nooskewl_wedge
cd c:\users\trent\code\nooskewl_wedge
git pull --rebase
goto done
:monster_rpg_3
cd c:\users\trent\code\monster_rpg_3
git pull --rebase
goto done
:done
endlocal
popd
