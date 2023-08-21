# Go (Inworld)
pushd .
cd ..

# Delete copied files
if [ -d "lib" ]; then
    rm -r lib
fi
if [ -d "src" ]; then
    rm -r src
fi
if [ -d "include" ]; then
    rm -r include
fi

# Go (inworld-ndk)
pushd .
cd ../../../inworld-ndk

# Delete build files
if [ -d "build" ]; then
    rm -r build
fi
if [ -d "build_Mac" ]; then
    rm -r build_Mac
fi
if [ -d "build_iOS" ]; then
    rm -r build_iOS
fi
if [ -d "build_Android" ]; then
    rm -r build_Android
fi

# Return (Inworld)
popd

# Return (dev-tools)
popd

echo SUCCESS
