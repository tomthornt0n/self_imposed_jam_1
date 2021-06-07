@echo off

if not exist build mkdir build

set compile_flags=/nologo /O2 /Zi
set sources="..\source\windows_jam_game_platform_impl.c" "..\source\jam_game.c"
set link_flags=/nologo /subsystem:windows /debug
set libs=resources.obj User32.lib Gdi32.lib ole32.lib
set output=jam_game.exe

echo ~~~~~~~~~~ buidling game ~~~~~~~~~~
pushd build
cl %compile_flags% %sources% /link %link_flags% %libs% /out:%output%
popd