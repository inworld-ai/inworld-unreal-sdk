if [ "$1" != "" ]; then
    PLATFORM="$1"
    if [ "$PLATFORM" != "Mac" ] && [ "$PLATFORM" != "iOS" ] && [ "$PLATFORM" != "Android" ] && [ "$PLATFORM" != "All" ]; then
        echo BAD PLATFORM "$PLATFORM"
        echo FAILURE
        exit 1
    fi
else
    PLATFORM="All"
fi

# Go (Inworld)
pushd .
cd ..

# Go (inworld-ndk)
pushd .
cd ../../../inworld-ndk

gen()
{
    # Generate first time
    if [ ! -d "build_$@" ]; then
        # Make build directory
        mkdir -p build_$@
    
        # Go (build)
        pushd .
        cd build_$@
        
        # Generate
        if [ "$@" == "Mac" ]; then
            cmake -G Xcode .. -DAEC=True -DMAC=True -DUE_DIR="/Users/Shared/Epic Games/UE_5.1/" -DINWORLD_LOG_CUSTOM=True -DCMAKE_OSX_ARCHITECTURES:STRING="x86_64;arm64"
        elif [ "$@" == "iOS" ]; then
            sudo xcode-select --switch /Applications/XCode_13.app/Contents/Developer
            cmake -G Xcode .. -DAEC=False -DIOS=True -DCMAKE_TOOLCHAIN_FILE=./ios.toolchain.cmake -DPLATFORM=OS64 -DINWORLD_LOG_CUSTOM=True
            sudo xcode-select --switch /Applications/XCode.app/Contents/Developer
        elif [ "$@" == "Android" ]; then
            cmake .. -DAEC=False -DANDROID=True -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=31 -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a -DCMAKE_ANDROID_NDK=/Users/mcarey/Library/Android/sdk/ndk/25.2.9519653 -DINWORLD_LOG_CUSTOM=True
        fi
        
        if [ $? -eq 0 ]; then
            echo Gen Success
        else
            echo Gen Failure
            echo FAILURE
            exit 1
        fi
        
        # Return (inworld-ndk)
        popd
    fi
}

build()
{
    # Go (build)
    pushd .
    cd build_$@
    
    # Build
    if [ "$@" == "iOS" ]; then
        sudo xcode-select --switch /Applications/XCode_13.app/Contents/Developer
    fi
    
    cmake --build . --target InworldNDK --config Release

    if [ $? -eq 0 ]; then
        echo Build Success
    else
        echo Build Failure
        echo FAILURE
        exit 1
    fi
    
    if [ "$@" == "iOS" ]; then
        sudo xcode-select --switch /Applications/XCode.app/Contents/Developer
    fi
    
    # Return (inworld-ndk)
    popd
}

if [ "$PLATFORM" == "Mac" ] || [ "$PLATFORM" == "All" ]; then
    gen Mac
    build mac
fi
if [ "$PLATFORM" == "iOS" ] || [ "$PLATFORM" == "All" ]; then
    gen iOS
    build iOS
fi
if [ "$PLATFORM" == "Android" ] || [ "$PLATFORM" == "All" ]; then
    gen Android
    build Android
fi

# Return (Inworld)
popd

# Copy lib
if [ "$PLATFORM" == "Mac" ] || [ "$PLATFORM" == "All" ]; then
    if [ ! -d "lib/Mac" ]; then
        mkdir -p lib/Mac
        cp -r "../../../inworld-ndk/build/Package/lib/Mac" "./lib"
    fi
fi
if [ "$PLATFORM" == "iOS" ] || [ "$PLATFORM" == "All" ]; then
    if [ ! -d "lib/iOS" ]; then
        mkdir -p lib/iOS
        cp -r "../../../inworld-ndk/build/Package/lib/iOS" "./lib"
    fi
fi
if [ "$PLATFORM" == "Android" ] || [ "$PLATFORM" == "All" ]; then
    if [ ! -d "lib/Android" ]; then
        mkdir -p lib/Android
        cp -r "../../../inworld-ndk/build/Package/lib/Android" "./lib"
    fi
fi

# Copy src
if [ ! -d "src" ]; then
    mkdir -p src
    cp -r "../../../inworld-ndk/build/Package/src" "./src"
fi

# Copy include
if [ ! -d "include" ]; then
    mkdir -p include
    cp -r "../../../inworld-ndk/build/Package/include" "./include"
fi

# Return (dev-tools)
popd

echo SUCCESS
