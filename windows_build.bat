@echo off

echo ~~~~~~~~~~ buidling windows layer ~~~~~~~~~~
pushd build
cl /nologo /O2 ..\source\jam_game_os_impl_windows.c ..\source\jam_game.c /Zi /link /nologo /subsystem:windows /debug User32.lib Gdi32.lib /out:jam_game.exe
popd