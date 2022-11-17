@echo off
pushd .
setlocal
if "%1"=="t" goto tgui3
if "%1"=="s" goto nooskewl_shim
if "%1"=="w" goto nooskewl_wedge
if "%1"=="m" goto monster_rpg_3
if "%1"=="d" goto data
:tgui3
cd c:\users\trent\code\monster-rpg-3\tgui3\build
if "%2"=="r" goto tgui3_release
rem copy relwithdebinfo\tgui3.dll ..\..\m3
goto done
:tgui3_release
rem copy release\tgui3.dll ..\..\m3
if "%3"=="s" goto tgui3_sign
goto done
:tgui3_sign
rem codesign c:\users\trent\code\m3\tgui3.dll
goto done
:nooskewl_shim
cd c:\users\trent\code\monster-rpg-3\Nooskewl_shim\build
if "%2"=="r" goto nooskewl_shim_release
rem copy relwithdebinfo\nooskewl_shim.dll ..\..\m3
goto done
:nooskewl_shim_release
rem copy release\nooskewl_shim.dll ..\..\m3
if "%3"=="s" goto nooskewl_shim_sign
goto done
:nooskewl_shim_sign
rem codesign c:\users\trent\code\m3\nooskewl_shim.dll
goto done
:nooskewl_wedge
cd c:\users\trent\code\monster-rpg-3\Nooskewl_wedge\build
if "%2"=="r" goto nooskewl_wedge_release
rem copy relwithdebinfo\nooskewl_wedge.dll ..\..\m3
goto done
:nooskewl_wedge_release
rem copy release\nooskewl_wedge.dll ..\..\m3
if "%3"=="s" goto nooskewl_wedge_sign
goto done
:nooskewl_wedge_sign
rem codesign c:\users\trent\code\m3\nooskewl_wedge.dll
goto done
:monster_rpg_3
cd c:\users\trent\code\monster-rpg-3\build
if "%2"=="r" goto monster_rpg_3_release
copy "relwithdebinfo\Monster RPG 3.exe" ..\..\m3
goto done
:monster_rpg_3_release
copy "release\Monster RPG 3.exe" ..\..\m3
if "%3"=="s" goto monster_rpg_3_sign
goto done
:monster_rpg_3_sign
if "%4"=="" goto sign_error
@codesign "c:\users\trent\code\m3\Monster RPG 3.exe" %4
goto done
:sign_error
del "C:\Users\trent\code\m3\Monster RPG 3.exe"
echo Forgot signing key password!
goto done
:data
if "%2"=="r" goto data_release
cd c:\users\trent\code\m3
xcopy /e /y ..\monster-rpg-3\data data\
copy "..\monster-rpg-3\docs\3rd_party.html" .
goto done
:data_release
cd c:\users\trent\code\monster-rpg-3\data
c:\users\trent\code\compress_dir\compress_dir.exe
move ..\data.cpa c:\users\trent\code\m3
copy "c:\users\trent\code\monster-rpg-3\docs\3rd_party.html" c:\users\trent\code\m3
goto done
:done
endlocal
popd
