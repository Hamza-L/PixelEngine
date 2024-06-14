@echo off
mklink /D ".\out\build\x64-Debug\shaders" "C:\Users\hlahm\Documents\gitProjects\PixelEngineNative\PixelEngine\shaders"
mklink /D ".\build\shaders" "C:\Users\hlahm\Documents\gitProjects\PixelEngineNative\PixelEngine\shaders"
mklink /h "compile_commands.json" "C:\Users\hlahm\Documents\gitProjects\PixelEngineNative\PixelEngine\build\compile_commands.json"
pause
