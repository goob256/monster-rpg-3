@echo off
pushd .
setlocal
if "%1"=="t" goto tgui3
if "%1"=="s" goto nooskewl_shim
if "%1"=="w" goto nooskewl_wedge
if "%1"=="m" goto monster_rpg_3
:tgui3
if "%2"=="r" goto tgui3_release
cd c:\users\trent\code\monster-rpg-3\tgui3\build
msbuild /p:configuration=relwithdebinfo tgui3.sln
goto done
:tgui3_release
cd c:\users\trent\code\monster-rpg-3\tgui3\build
msbuild /p:configuration=release tgui3.sln
goto done
:nooskewl_shim
if "%2"=="r" goto nooskewl_shim_release
cd c:\users\trent\code\monster-rpg-3\Nooskewl_shim\build
msbuild /p:configuration=relwithdebinfo nooskewl_shim.sln
goto done
:nooskewl_shim_release
cd c:\users\trent\code\monster-rpg-3\Nooskewl_shim\build
msbuild /p:configuration=release nooskewl_shim.sln
goto done
:nooskewl_wedge
if "%2"=="r" goto nooskewl_wedge_release
cd c:\users\trent\code\monster-rpg-3\Nooskewl_Wedge\build
msbuild /p:configuration=relwithdebinfo Nooskewl_Wedge.sln
goto done
:nooskewl_wedge_release
cd c:\users\trent\code\monster-rpg-3\Nooskewl_Wedge\build
msbuild /p:configuration=release Nooskewl_Wedge.sln
goto done
:monster_rpg_3
if "%2"=="r" goto monster_rpg_3_release
cd c:\users\trent\code\monster-rpg-3\build
msbuild /p:configuration=relwithdebinfo "MonsterRPG3.sln"
goto done
:monster_rpg_3_release
cd c:\users\trent\code\monster-rpg-3\build
msbuild /p:configuration=release "MonsterRPG3.sln"
goto done
:done
endlocal
popd
