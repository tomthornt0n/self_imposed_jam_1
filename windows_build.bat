@echo off

if not exist build mkdir build

call .\windows_build_resources.bat

echo ~~~~~~~~~~ buidling game ~~~~~~~~~~
pushd build
cl /nologo /O2 ..\source\jam_game_os_impl_windows.c ..\source\jam_game.c /Zi /arch:AVX /link /nologo /subsystem:windows /debug resources.obj User32.lib Gdi32.lib /out:jam_game.exe
popd