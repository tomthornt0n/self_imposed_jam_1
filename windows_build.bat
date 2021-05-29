@echo off

if not exist build mkdir build

echo ~~~~~~~~~~ buidling game ~~~~~~~~~~
pushd build
cl /nologo /O2 /Zi ..\source\windows_jam_game_platform_impl.c ..\source\jam_game.c /link /nologo /subsystem:windows /debug resources.obj User32.lib Gdi32.lib /out:jam_game.exe
popd