import os
import sys
import stat
import getopt
import shutil
import contextlib

os.chdir(os.path.dirname(os.path.abspath(__file__)))

ndk_path = os.path.join(os.getcwd(), '../InworldAI/inworld-ndk')
build_path = os.path.join(os.getcwd(), '../InworldAI/inworld-ndk/build')
package_path = os.path.join(os.getcwd(), '../InworldAI/inworld-ndk/build/package')
copy_path = os.path.join(os.getcwd(), './../InworldAI/Source/ThirdParty/InworldAINDKLibrary/')

print(ndk_path)

@contextlib.contextmanager
def in_path(path):
    prev_dir = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(prev_dir)

class BuildConfiguration:
    def __init__(self, gen, build):
        self.gen = gen
        self.build = build

build_configurations = {
    'Win64': BuildConfiguration(
        ['cmake .. -G "Visual Studio 16 2019" -DINWORLD_SHARED=False -DAEC=True -DINWORLD_LOG_CALLBACK=True -DINWORLD_AUDIO_DUMP=True'],
        ['cmake --build . --target inworld-ndk --config Release']
    ),
    'Win64-ndk-dll': BuildConfiguration(
        ['cmake .. -G "Visual Studio 16 2019" -DINWORLD_SHARED=True -DAEC=True -DINWORLD_LOG_CALLBACK=True -DINWORLD_AUDIO_DUMP=True'],
        ['cmake --build . --target inworld-ndk --config Release']
    ),
    'Mac': BuildConfiguration(
        ['cmake .. -DINWORLD_SHARED=False -DAEC=False -DMAC=True -DINWORLD_LOG_CALLBACK=True -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" -DINWORLD_AUDIO_DUMP=True'],
        ['cmake --build . --target inworld-ndk --config Release']
    ),
    'iOS': BuildConfiguration(
        ['cmake -G Xcode .. -DINWORLD_SHARED=False -DAEC=False -DIOS=True -DCMAKE_TOOLCHAIN_FILE=./ios.toolchain.cmake -DPLATFORM=OS64 -DINWORLD_LOG_CALLBACK=True'],
        ['cmake --build . --target inworld-ndk --config Release -- CODE_SIGNING_ALLOWED=NO']
    ),
    'Android': BuildConfiguration(
        ['cmake .. -DINWORLD_SHARED=False -DAEC=False -DANDROID=True -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=31 -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a -DCMAKE_ANDROID_NDK=/Users/runner/Library/Android/sdk/ndk/25.2.9519653 -DINWORLD_LOG_CALLBACK=True'],
        ['cmake --build . --target inworld-ndk --config Release']
    )
}

platform = ''
clean = False
build = False
copy = False

def usage():
    print('[-p=, --platform=] [Win64, Mac, iOS, Android]')
    print('[-c, --clean]')
    print('[-b, --build]')
    print('[-x, --copy]')

try:
    opts, values = getopt.getopt(sys.argv[1:], "hp:cbx", ['help', 'platform=', 'clean', 'build', 'copy'])
except getopt.GetoptError as err:
    print(err)
    usage()
    raise SystemExit('Error: Unknown Argument.')

for opt, val in opts:
    if opt in ('-h', '--help'):
        usage()
        sys.exit()
    elif opt in ('-p', '--platform'):
        platform = val
    elif opt in ('-c', '--clean'):
        clean = True
    elif opt in ('-b', '--build'):
        build = True
    elif opt in ('-x', '--copy'):
        copy = True

if clean:
    def del_dir(path):
        if os.path.exists(path):
            for root, dirs, files in os.walk(path, topdown=False):
                for name in files:
                    filename = os.path.join(root, name)
                    os.chmod(filename, stat.S_IWUSR)
                    os.remove(filename)
                for name in dirs:
                    os.rmdir(os.path.join(root, name))
            os.rmdir(path)  

    with in_path(copy_path):
        dirs_to_del = ['src', 'lib', 'include']
        for dir_to_del in dirs_to_del:
            del_dir(os.path.join(os.getcwd(), dir_to_del))
            
    with in_path(ndk_path):
        del_dir(os.path.join(os.getcwd(), 'build'))

if build:
    if not platform in build_configurations.keys():
        usage()
        raise SystemExit('Error (build): Missing platform.')

    if not os.path.exists(build_path):
        os.mkdir(build_path)
        with in_path(build_path):
            for command in build_configurations[platform].gen:
                if os.system(command) != 0:
                    raise SystemExit('Error (build): Unable to generate NDK.')
     
    with in_path(build_path):
        for command in build_configurations[platform].build:
            if os.system(command) != 0:
                raise SystemExit('Error (build): Unable to build NDK.')

if copy:
    if not os.path.exists(package_path):
        usage()
        raise SystemExit('Error (copy): Unable to copy NDK files.')

    dirs_to_copy = ['lib', 'include', 'src']
    orig_dirs = {}
    copy_dirs = {}

    with in_path(package_path):
        for dir_to_copy in dirs_to_copy:
            orig_dirs[dir_to_copy] = os.path.join(os.getcwd(), dir_to_copy)
        
    with in_path(copy_path):
        for dir_to_copy in dirs_to_copy:
            copy_dirs[dir_to_copy] = os.path.join(os.getcwd(), dir_to_copy)

    for dir_to_copy in dirs_to_copy:
        shutil.copytree(orig_dirs[dir_to_copy], copy_dirs[dir_to_copy], dirs_exist_ok=True)
