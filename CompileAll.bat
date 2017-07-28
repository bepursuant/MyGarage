@echo off

echo Begin asset compilation...
echo:

echo   Removing old assets...
del Assets.h
echo   ok!
echo:

echo   Compiling new assets...
CompileAssets.exe Portal.html assets_portal
echo   ok!
echo:

echo done!
rem pause