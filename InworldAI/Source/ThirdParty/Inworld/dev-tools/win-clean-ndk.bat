@echo off

:: Go (Inworld)
pushd .
cd ..

:: Delete copied files
if exist src (rd /s /q src)
if exist lib (rd /s /q lib)
if exist include (rd /s /q include)

:: Go (inworld-ndk)
pushd .
cd ../../../inworld-ndk

:: Delete build files
if exist build (rd /s /q build)

:: Return (Inworld)
popd

:: Return (dev-tools)
popd

echo SUCCESS
