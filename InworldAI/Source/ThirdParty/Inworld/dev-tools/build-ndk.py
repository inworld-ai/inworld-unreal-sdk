import os
import sys
import getopt
import shutil
import contextlib

os.chdir(os.path.dirname(os.path.abspath(__file__)))

ndk_path = '../../../../inworld-ndk'
cpy_path = '..'

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
        ['cmake .. -DAEC=True -DUE_DIR="C:/Program Files/Epic Games/UE_5.1" -DINWORLD_LOG_CUSTOM=True'],
        ['cmake --build . --target InworldNDKApp --config Release']
    ),
    'Mac': BuildConfiguration([''],['']),
    'iOS': BuildConfiguration([''],['']),
    'Android': BuildConfiguration([''],[''])
}
platform = ''

def usage():
    print('--platform=[Win64, Mac, iOS, Android]')

try:
    opts, values = getopt.getopt(sys.argv[1:], "hp:", ["help", "platform="])
except getopt.GetoptError as err:
    print(err)
    usage()
    raise SystemExit('Error: Unknown Argument.')

for opt, val in opts:
    if opt in ("-h", "--help"):
        usage()
        sys.exit()
    elif opt in ("-p", "--platform"):
        platform = val

if not platform in build_configurations.keys():
    usage()
    raise SystemExit('Error: Missing Platform.')

with in_path(ndk_path):
        build_dir = os.path.join(os.getcwd(), 'build')
        if not os.path.exists(build_dir):
            os.mkdir(build_dir)
            with in_path('build'):
                for command in build_configurations[platform].gen:
                    if os.system(command) != 0:
                        raise SystemExit('Error: Unable to generate NDK.')
        with in_path('build'):
            for command in build_configurations[platform].build:
                if os.system(command) != 0:
                    raise SystemExit('Error: Unable to build NDK.')

with in_path(ndk_path):
    orig_lib_dir = os.path.join(os.getcwd(), 'build', 'Package', 'lib', platform)
    orig_inc_dir = os.path.join(os.getcwd(), 'build', 'Package', 'include')
    orig_src_dir = os.path.join(os.getcwd(), 'build', 'Package', 'src')
    
with in_path(cpy_path):
    os.mkdir('lib')
    with in_path('lib'):
        os.mkdir(platform)
    os.mkdir('include')
    os.mkdir('src')
    copy_lib_dir = os.path.join(os.getcwd(), 'lib', platform)
    copy_inc_dir = os.path.join(os.getcwd(), 'include')
    copy_src_dir = os.path.join(os.getcwd(), 'src')

shutil.copytree(orig_lib_dir, copy_lib_dir, dirs_exist_ok=True)
shutil.copytree(orig_inc_dir, copy_inc_dir, dirs_exist_ok=True)
shutil.copytree(orig_src_dir, copy_src_dir, dirs_exist_ok=True)

'''
for platform in build_platforms:
    orig_lib_dir = ""
    new_dir = ""
    with in_path(ndk_path):
        orig_lib_dir = os.path.join(os.getcwd(), 'build', platform, )
    new_dir
    shutil.copytree(CurDir + Subdir, ReleaseDir + Subdir, dirs_exist_ok=True)
'''