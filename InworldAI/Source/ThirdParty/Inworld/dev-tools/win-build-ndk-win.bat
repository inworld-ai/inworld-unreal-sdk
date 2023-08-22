@echo off

:: Go (Inworld)
pushd .
cd ..

:: Go (inworld-ndk)
pushd .
cd ../../../inworld-ndk

:: Generate first time
if not exist build\ (
	:: Make (build)
	mkdir build

	:: Go (build)
	pushd .
	cd build
	
	:: Generate
	cmake .. -DAEC=True -DUE_DIR="C:/Program Files/Epic Games/UE_5.1" -DINWORLD_LOG_CUSTOM=True
	
	:: Exit on generate failure
	if errorlevel 1 (
		echo FAILED
		exit /B
	)
	
	:: Return (inworld-ndk)
	popd
)

:: Go (build)
pushd .
cd build

:: Build
cmake --build . --target InworldNDKApp --config Release

:: Exit on build failure
if errorlevel 1 (
	echo FAILED
	exit /B
)

:: Return (inworld-ndk)
popd

:: Return (Inworld)
popd

:: Copy lib
if not exist lib (mkdir lib)
if not exist lib\Win64 (mkdir lib\Win64)
robocopy "..\..\..\inworld-ndk\build\Package\lib\Win64" ".\lib\Win64" /E

:: Copy src
if not exist src (mkdir src)
robocopy "..\..\..\inworld-ndk\build\Package\src" ".\src" /E

:: Copy include
if not exist include (mkdir include)
robocopy "..\..\..\inworld-ndk\build\Package\include" ".\include" /E

:: Return (dev-tools)
popd

echo SUCCESS

pause
