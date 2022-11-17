@echo off
pushd .
setlocal
if "%2"=="r" goto release
set FLAGS="-DDEBUG=on"
:release
if "%3"=="steam" goto steam
rem above line was commented out, but I'm doing a non-Steam build
rem always steam now
rem goto steam
goto go
:steam
set FLAGS="-DSTEAMWORKS=on"
:go
if "%1"=="t" goto tgui3
if "%1"=="s" goto nooskewl_shim
if "%1"=="w" goto nooskewl_wedge
if "%1"=="m" goto monster_rpg_3
if "%1"=="d" goto data
:tgui3
del c:\users\trent\code\m3\tgui3.dll
cd c:\users\trent\code\monster-rpg-3\tgui3
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\monster-rpg-3\tools\batch_files\cmake\windows\tgui3.bat %FLAGS%
goto done
:nooskewl_shim
del c:\users\trent\code\m3\nooskewl_shim.dll
cd c:\users\trent\code\monster-rpg-3\Nooskewl_shim
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\monster-rpg-3\tools\batch_files\cmake\windows\nooskewl_shim.bat %FLAGS%
goto done
:nooskewl_wedge
del c:\users\trent\code\m3\nooskewl_wedge.dll
cd c:\users\trent\code\monster-rpg-3\Nooskewl_shim
cd c:\users\trent\code\monster-rpg-3\Nooskewl_wedge
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\monster-rpg-3\tools\batch_files\cmake\windows\nooskewl_wedge.bat %FLAGS%
goto done
:monster_rpg_3
del "c:\users\trent\code\m3\Monster RPG 3.exe"
cd c:\users\trent\code\monster-rpg-3
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\monster-rpg-3\tools\batch_files\cmake\windows\monster-rpg-3.bat %FLAGS%
goto done
:data
rmdir /s /q c:\users\trent\code\m3\data
del c:\users\trent\code\m3\data.cpa
cd c:\users\trent\code\m3
rmdir /s /q data
goto done
:done
endlocal
popd
