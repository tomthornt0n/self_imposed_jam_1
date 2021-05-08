@echo off

pushd build

echo ~~~~~~~~~~ generating resource C files ~~~~~~~~~~
.\resembed.exe

echo ~~~~~~~~~~~~~~ compiling resources ~~~~~~~~~~~~~~
cl /nologo resources.gen.c /c /Fo:resources.obj

popd